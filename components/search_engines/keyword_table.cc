// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/search_engines/keyword_table.h"

#include <stddef.h>

#include <memory>
#include <set>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "components/history/core/browser/url_database.h"
#include "components/search_engines/search_terms_data.h"
#include "components/search_engines/template_url.h"
#include "components/webdata/common/web_database.h"
#include "sql/statement.h"
#include "sql/transaction.h"
#include "url/gurl.h"

using base::Time;

// static
const char KeywordTable::kDefaultSearchProviderKey[] =
    "Default Search Provider ID";

namespace {

// Keys used in the meta table.
const char kBuiltinKeywordVersion[] = "Builtin Keyword Version";

const std::string ColumnsForVersion(int version, bool concatenated) {
  std::vector<std::string> columns;

  columns.push_back("id");
  columns.push_back("short_name");
  columns.push_back("keyword");
  columns.push_back("favicon_url");
  columns.push_back("url");
  columns.push_back("safe_for_autoreplace");
  columns.push_back("originating_url");
  columns.push_back("date_created");
  columns.push_back("usage_count");
  columns.push_back("input_encodings");
  if (version <= 67) {
    // Column removed after version 67.
    columns.push_back("show_in_default_list");
  }
  columns.push_back("suggest_url");
  columns.push_back("prepopulate_id");
  if (version <= 44) {
    // Columns removed after version 44.
    columns.push_back("autogenerate_keyword");
    columns.push_back("logo_id");
  }
  columns.push_back("created_by_policy");
  columns.push_back("instant_url");
  columns.push_back("last_modified");
  columns.push_back("sync_guid");
  if (version >= 47) {
    // Column added in version 47.
    columns.push_back("alternate_urls");
  }
  if (version >= 49) {
    // Column added in version 49.
    columns.push_back("search_terms_replacement_key");
  }
  if (version >= 52) {
    // Column added in version 52.
    columns.push_back("image_url");
    columns.push_back("search_url_post_params");
    columns.push_back("suggest_url_post_params");
    columns.push_back("instant_url_post_params");
    columns.push_back("image_url_post_params");
  }
  if (version >= 53) {
    // Column added in version 53.
    columns.push_back("new_tab_url");
  }
  if (version >= 69) {
    // Column added in version 69.
    columns.push_back("last_visited");
  }

  return base::JoinString(columns, std::string(concatenated ? " || " : ", "));
}


// Inserts the data from |data| into |s|.  |s| is assumed to have slots for all
// the columns in the keyword table.  |id_column| is the slot number to bind
// |data|'s |id| to; |starting_column| is the slot number of the first of a
// contiguous set of slots to bind all the other fields to.
void BindURLToStatement(const TemplateURLData& data,
                        sql::Statement* s,
                        int id_column,
                        int starting_column) {
  // Serialize |alternate_urls| to JSON.
  // TODO(beaudoin): Check what it would take to use a new table to store
  // alternate_urls while keeping backups and table signature in a good state.
  // See: crbug.com/153520
  base::ListValue alternate_urls_value;
  for (size_t i = 0; i < data.alternate_urls.size(); ++i)
    alternate_urls_value.AppendString(data.alternate_urls[i]);
  std::string alternate_urls;
  base::JSONWriter::Write(alternate_urls_value, &alternate_urls);

  s->BindInt64(id_column, data.id);
  s->BindString16(starting_column, data.short_name());
  s->BindString16(starting_column + 1, data.keyword());
  s->BindString(starting_column + 2, data.favicon_url.is_valid() ?
      history::URLDatabase::GURLToDatabaseURL(data.favicon_url) :
      std::string());
  s->BindString(starting_column + 3, data.url());
  s->BindBool(starting_column + 4, data.safe_for_autoreplace);
  s->BindString(starting_column + 5, data.originating_url.is_valid() ?
      history::URLDatabase::GURLToDatabaseURL(data.originating_url) :
      std::string());
  s->BindInt64(starting_column + 6, data.date_created.ToTimeT());
  s->BindInt(starting_column + 7, data.usage_count);
  s->BindString(starting_column + 8,
                base::JoinString(data.input_encodings, ";"));
  s->BindString(starting_column + 9, data.suggestions_url);
  s->BindInt(starting_column + 10, data.prepopulate_id);
  s->BindBool(starting_column + 11, data.created_by_policy);
  s->BindString(starting_column + 12, data.instant_url);
  s->BindInt64(starting_column + 13, data.last_modified.ToTimeT());
  s->BindString(starting_column + 14, data.sync_guid);
  s->BindString(starting_column + 15, alternate_urls);
  s->BindString(starting_column + 16, data.search_terms_replacement_key);
  s->BindString(starting_column + 17, data.image_url);
  s->BindString(starting_column + 18, data.search_url_post_params);
  s->BindString(starting_column + 19, data.suggestions_url_post_params);
  s->BindString(starting_column + 20, data.instant_url_post_params);
  s->BindString(starting_column + 21, data.image_url_post_params);
  s->BindString(starting_column + 22, data.new_tab_url);
  s->BindInt64(starting_column + 23, data.last_visited.ToTimeT());
}

WebDatabaseTable::TypeKey GetKey() {
  // We just need a unique constant. Use the address of a static that
  // COMDAT folding won't touch in an optimizing linker.
  static int table_key = 0;
  return reinterpret_cast<void*>(&table_key);
}

}  // namespace

KeywordTable::KeywordTable() {
}

KeywordTable::~KeywordTable() {}

KeywordTable* KeywordTable::FromWebDatabase(WebDatabase* db) {
  return static_cast<KeywordTable*>(db->GetTable(GetKey()));
}

WebDatabaseTable::TypeKey KeywordTable::GetTypeKey() const {
  return GetKey();
}

bool KeywordTable::CreateTablesIfNecessary() {
  return db_->DoesTableExist("keywords") ||
      db_->Execute("CREATE TABLE keywords ("
          "id INTEGER PRIMARY KEY,"
          "short_name VARCHAR NOT NULL,"
          "keyword VARCHAR NOT NULL,"
          "favicon_url VARCHAR NOT NULL,"
          "url VARCHAR NOT NULL,"
          "safe_for_autoreplace INTEGER,"
          "originating_url VARCHAR,"
          "date_created INTEGER DEFAULT 0,"
          "usage_count INTEGER DEFAULT 0,"
          "input_encodings VARCHAR,"
          "suggest_url VARCHAR,"
          "prepopulate_id INTEGER DEFAULT 0,"
          "created_by_policy INTEGER DEFAULT 0,"
          "instant_url VARCHAR,"
          "last_modified INTEGER DEFAULT 0,"
          "sync_guid VARCHAR,"
          "alternate_urls VARCHAR,"
          "search_terms_replacement_key VARCHAR,"
          "image_url VARCHAR,"
          "search_url_post_params VARCHAR,"
          "suggest_url_post_params VARCHAR,"
          "instant_url_post_params VARCHAR,"
          "image_url_post_params VARCHAR,"
          "new_tab_url VARCHAR,"
          " last_visited INTEGER DEFAULT 0)");
}

bool KeywordTable::IsSyncable() {
  return true;
}

bool KeywordTable::MigrateToVersion(int version,
                                    bool* update_compatible_version) {
  // Migrate if necessary.
  switch (version) {
    case 53:
      *update_compatible_version = true;
      return MigrateToVersion53AddNewTabURLColumn();
    case 59:
      *update_compatible_version = true;
      return MigrateToVersion59RemoveExtensionKeywords();
    case 68:
      *update_compatible_version = true;
      return MigrateToVersion68RemoveShowInDefaultListColumn();
    case 69:
      return MigrateToVersion69AddLastVisitedColumn();
  }

  return true;
}

bool KeywordTable::PerformOperations(const Operations& operations) {
  sql::Transaction transaction(db_);
  if (!transaction.Begin())
    return false;

  for (Operations::const_iterator i(operations.begin()); i != operations.end();
       ++i) {
    switch (i->first) {
      case ADD:
        if (!AddKeyword(i->second))
          return false;
        break;

      case REMOVE:
        if (!RemoveKeyword(i->second.id))
          return false;
        break;

      case UPDATE:
        if (!UpdateKeyword(i->second))
          return false;
        break;
    }
  }

  return transaction.Commit();
}

bool KeywordTable::GetKeywords(Keywords* keywords) {
  std::string query("SELECT " + GetKeywordColumns() +
                    " FROM keywords ORDER BY id ASC");
  sql::Statement s(db_->GetUniqueStatement(query.c_str()));

  std::set<TemplateURLID> bad_entries;
  while (s.Step()) {
    keywords->push_back(TemplateURLData());
    if (!GetKeywordDataFromStatement(s, &keywords->back())) {
      bad_entries.insert(s.ColumnInt64(0));
      keywords->pop_back();
    }
  }
  bool succeeded = s.Succeeded();
  for (std::set<TemplateURLID>::const_iterator i(bad_entries.begin());
       i != bad_entries.end(); ++i)
    succeeded &= RemoveKeyword(*i);
  return succeeded;
}

bool KeywordTable::SetDefaultSearchProviderID(int64_t id) {
  return meta_table_->SetValue(kDefaultSearchProviderKey, id);
}

int64_t KeywordTable::GetDefaultSearchProviderID() {
  int64_t value = kInvalidTemplateURLID;
  meta_table_->GetValue(kDefaultSearchProviderKey, &value);
  return value;
}

bool KeywordTable::SetBuiltinKeywordVersion(int version) {
  return meta_table_->SetValue(kBuiltinKeywordVersion, version);
}

int KeywordTable::GetBuiltinKeywordVersion() {
  int version = 0;
  return meta_table_->GetValue(kBuiltinKeywordVersion, &version) ? version : 0;
}

// static
std::string KeywordTable::GetKeywordColumns() {
  return ColumnsForVersion(WebDatabase::kCurrentVersionNumber, false);
}

bool KeywordTable::MigrateToVersion53AddNewTabURLColumn() {
  return db_->Execute("ALTER TABLE keywords ADD COLUMN new_tab_url "
                      "VARCHAR DEFAULT ''");
}

bool KeywordTable::MigrateToVersion59RemoveExtensionKeywords() {
  return db_->Execute("DELETE FROM keywords "
                      "WHERE url LIKE 'chrome-extension://%'");
}

// SQLite does not support DROP COLUMN operation. So A new table is created
// without the show_in_default_list column. Data from all but the dropped column
// of the old table is copied into it. After that, the old table is dropped and
// the new table is renamed to it.
bool KeywordTable::MigrateToVersion68RemoveShowInDefaultListColumn() {
  sql::Transaction transaction(db_);
  std::string query_str =
      std::string("INSERT INTO temp_keywords SELECT " +
                  ColumnsForVersion(68, false) + " FROM keywords");
  const char* clone_query = query_str.c_str();
  return transaction.Begin() &&
         db_->Execute(
             "CREATE TABLE temp_keywords ("
             "id INTEGER PRIMARY KEY,"
             "short_name VARCHAR NOT NULL,"
             "keyword VARCHAR NOT NULL,"
             "favicon_url VARCHAR NOT NULL,"
             "url VARCHAR NOT NULL,"
             "safe_for_autoreplace INTEGER,"
             "originating_url VARCHAR,"
             "date_created INTEGER DEFAULT 0,"
             "usage_count INTEGER DEFAULT 0,"
             "input_encodings VARCHAR,"
             "suggest_url VARCHAR,"
             "prepopulate_id INTEGER DEFAULT 0,"
             "created_by_policy INTEGER DEFAULT 0,"
             "instant_url VARCHAR,"
             "last_modified INTEGER DEFAULT 0,"
             "sync_guid VARCHAR,"
             "alternate_urls VARCHAR,"
             "search_terms_replacement_key VARCHAR,"
             "image_url VARCHAR,"
             "search_url_post_params VARCHAR,"
             "suggest_url_post_params VARCHAR,"
             "instant_url_post_params VARCHAR,"
             "image_url_post_params VARCHAR,"
             "new_tab_url VARCHAR)") &&
         db_->Execute(clone_query) && db_->Execute("DROP TABLE keywords") &&
         db_->Execute("ALTER TABLE temp_keywords RENAME TO keywords") &&
         transaction.Commit();
}

bool KeywordTable::MigrateToVersion69AddLastVisitedColumn() {
  return db_->Execute("ALTER TABLE keywords ADD COLUMN last_visited "
                      "INTEGER DEFAULT 0");
}

// static
bool KeywordTable::GetKeywordDataFromStatement(const sql::Statement& s,
                                               TemplateURLData* data) {
  DCHECK(data);

  data->SetShortName(s.ColumnString16(1));
  data->SetKeyword(s.ColumnString16(2));
  // Due to past bugs, we might have persisted entries with empty URLs.  Avoid
  // reading these out.  (GetKeywords() will delete these entries on return.)
  // NOTE: This code should only be needed as long as we might be reading such
  // potentially-old data and can be removed afterward.
  if (s.ColumnString(4).empty())
    return false;
  data->SetURL(s.ColumnString(4));
  data->suggestions_url = s.ColumnString(10);
  data->instant_url = s.ColumnString(13);
  data->image_url = s.ColumnString(18);
  data->new_tab_url = s.ColumnString(23);
  data->search_url_post_params = s.ColumnString(19);
  data->suggestions_url_post_params = s.ColumnString(20);
  data->instant_url_post_params = s.ColumnString(21);
  data->image_url_post_params = s.ColumnString(22);
  data->favicon_url = GURL(s.ColumnString(3));
  data->originating_url = GURL(s.ColumnString(6));
  data->safe_for_autoreplace = s.ColumnBool(5);
  data->input_encodings = base::SplitString(
      s.ColumnString(9), ";", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
  data->id = s.ColumnInt64(0);
  data->date_created = Time::FromTimeT(s.ColumnInt64(7));
  data->last_modified = Time::FromTimeT(s.ColumnInt64(14));
  data->created_by_policy = s.ColumnBool(12);
  data->usage_count = s.ColumnInt(8);
  data->prepopulate_id = s.ColumnInt(11);
  data->sync_guid = s.ColumnString(15);

  data->alternate_urls.clear();
  base::JSONReader json_reader;
  std::unique_ptr<base::Value> value(
      json_reader.ReadToValue(s.ColumnString(16)));
  base::ListValue* alternate_urls_value;
  if (value.get() && value->GetAsList(&alternate_urls_value)) {
    std::string alternate_url;
    for (size_t i = 0; i < alternate_urls_value->GetSize(); ++i) {
      if (alternate_urls_value->GetString(i, &alternate_url))
        data->alternate_urls.push_back(alternate_url);
    }
  }

  data->search_terms_replacement_key = s.ColumnString(17);
  data->last_visited = Time::FromTimeT(s.ColumnInt64(24));

  return true;
}

bool KeywordTable::AddKeyword(const TemplateURLData& data) {
  DCHECK(data.id);
  std::string query("INSERT INTO keywords (" + GetKeywordColumns() + ") "
                    "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,"
                    "        ?,?)");
  sql::Statement s(db_->GetCachedStatement(SQL_FROM_HERE, query.c_str()));
  BindURLToStatement(data, &s, 0, 1);

  return s.Run();
}

bool KeywordTable::RemoveKeyword(TemplateURLID id) {
  DCHECK(id);
  sql::Statement s(db_->GetCachedStatement(
      SQL_FROM_HERE, "DELETE FROM keywords WHERE id = ?"));
  s.BindInt64(0, id);

  return s.Run();
}

bool KeywordTable::UpdateKeyword(const TemplateURLData& data) {
  DCHECK(data.id);
  sql::Statement s(db_->GetCachedStatement(
      SQL_FROM_HERE,
      "UPDATE keywords SET short_name=?, keyword=?, favicon_url=?, url=?, "
      "safe_for_autoreplace=?, originating_url=?, date_created=?, "
      "usage_count=?, input_encodings=?, suggest_url=?, "
      "prepopulate_id=?, created_by_policy=?, instant_url=?, "
      "last_modified=?, sync_guid=?, alternate_urls=?, "
      "search_terms_replacement_key=?, image_url=?, search_url_post_params=?, "
      "suggest_url_post_params=?, instant_url_post_params=?, "
      "image_url_post_params=?, new_tab_url=?, last_visited=? WHERE id=?"));
  BindURLToStatement(data, &s, 24, 0);  // "24" binds id() as the last item.

  return s.Run();
}

bool KeywordTable::GetKeywordAsString(TemplateURLID id,
                                      const std::string& table_name,
                                      std::string* result) {
  std::string query("SELECT " +
      ColumnsForVersion(WebDatabase::kCurrentVersionNumber, true) +
      " FROM " + table_name + " WHERE id=?");
  sql::Statement s(db_->GetUniqueStatement(query.c_str()));
  s.BindInt64(0, id);

  if (!s.Step()) {
    LOG_IF(WARNING, s.Succeeded()) << "No keyword with id: " << id
                                   << ", ignoring.";
    return true;
  }

  if (!s.Succeeded())
    return false;

  *result = s.ColumnString(0);
  return true;
}
