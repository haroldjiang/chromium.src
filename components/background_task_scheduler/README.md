# Quick Start Guide to using BackgroundTaskScheduler

## Background

In Android M+ it is encouraged to use `JobScheduler` for all background jobs,
instead of using things like `IntentService` or polling using alarms. Using the
system API is beneficial as it has a full view of what goes on in the system and
can schedule jobs accordingly.

However, this functionality was introduced in Android L, and
the API has been very stable since Android M. This means that we also need a
similar framework for older versions of Android, which is provided by
Google Play services. We prefer system APIs, since they do not require including
external libraries which bloats the APK size of Chrome and adds unnecessary
complexity. The GcmNetworkManager is only used when the system API is not
available.

The `background_task_scheduler` component provides a new framework for use
within chromium to schedule and execute background jobs using the frameworks
available on a given version of Android.
The public API of the framework is similar to that of the Android
`JobScheduler`, but it is backed by either the system `JobScheduler` API or by
GcmNetworkManager. What service is used to back the framework remains unknown to
callers of the API.

## What is a task

A task is defined as a class that implements the `BackgroundTask` interface,
which looks like this:

```java
interface BackgroundTask {
  interface TaskFinishedCallback {
    void taskFinished(boolean needsReschedule);
  }

  boolean onStartTask(Context context,
                      TaskParameters taskParameters,
                      TaskFinishedCallback callback);
  boolean onStopTask(Context context,
                     TaskParameters taskParameters);
}
```

**Any class implementing this interface must have a public constructor which takes
no arguments.**

A task must also have a unique ID, and it must be listed in `TaskIds` to ensure
there is no overlap between different tasks.

## How to schedule a task

A task is scheduled by creating an object containing information about the task,
such as when to run it, whether it requires battery, and other similar
constraints. This object is called `TaskInfo` and has a builder you can use
to set all the relevant fields.

There are two main types of tasks; one-off tasks and periodic tasks. One-off
tasks are only executed once, whereas periodic tasks are executed once per
a defined interval.

As an example for how to create a one-off task that executes in 200 minutes,
you can do the following:

```java
TaskInfo.createOneOffTask(TaskIds.YOUR_FEATURE,
                            MyBackgroundTask.class,
                            TimeUnit.MINUTES.toMillis(200)).build();
```

For a periodic task that executes every 200 minutes, you can call:

```java
TaskInfo.createPeriodicTask(TaskIds.YOUR_FEATURE,
                              MyBackgroundTask.class,
                              TimeUnit.MINUTES.toMillis(200)).build();
```

Typically you will also set other required parameters such as what type of
network conditions are necessary and whether the task requires the device to
be charging. They can be set on the builder like this:

```java
TaskInfo.createOneOffTask(TaskIds.YOUR_FEATURE,
                            MyBackgroundTask.class,
                            TimeUnit.MINUTES.toMillis(100)
                            TimeUnit.MINUTES.toMillis(200))
                          .setRequiresCharging(true)
                          .setRequiredNetworkType(TaskInfo.NETWORK_TYPE_UNMETERED)
                          .build();
```

When the task is ready for scheduling, you use the
`BackgroundTaskSchedulerFactory` to get the current instance of the
`BackgroundTaskScheduler` and use it to schedule the job.

```java
BackgroundTaskScheduleFactory.getScheduler().schedule(myTaskInfo);
```

If you ever need to cancel a task, you can do that by calling `cancel`, and
passing in the task ID:

```java
BackgroundTaskScheduleFactory.getScheduler().cancel(TaskIds.YOUR_FEATURE);
```

## Passing task arguments

A `TaskInfo` supports passing in arguments through a `Bundle`, but only values
that can be part of an Android `BaseBundle` are allowed. You can pass them in
using the `TaskInfo.Builder`:

```java
Bundle myBundle = new Bundle();
myBundle.putString("foo", "bar");
myBundle.putLong("number", 1337L);

TaskInfo.createOneOffTask(TaskIds.YOUR_FEATURE,
                            MyBackgroundTask.class,
                            TimeUnit.MINUTES.toMillis(100)
                            TimeUnit.MINUTES.toMillis(200))
                          .setExtras(myBundle)
                          .build();
```

These arguments will be readable for the task through the `TaskParameters`
object that is passed to both `onStartTask(...)` and `onStopTask(...)`, by
doing the following:

```java
boolean onStartTask(Context context,
                    TaskParameters taskParameters,
                    TaskFinishedCallback callback) {
  Bundle myExtras = taskParameters.getExtras();
  // Use |myExtras|.
  ...
}
```

## Background processing

Even though the `BackgroundTaskScheduler` provides functionality for invoking
code while the application is in the background, the `BackgroundTask` instance
is still invoked on the application main thread.

This means that unless the operation is extremely quick, processing must happen
asynchronously, and the call to `onStartJob(...)` must return before the task
has finished processing. In that case, `onStartJob(...)` must return true, and
instead invoke the `TaskFinishedCallback` when the processing is finished, which
typically happens on a different `Thread`, `Handler` or using an `AsyncTask`.

If the task finishes while still being on the main thread, `onStartJob(...)`
should return false, indicating that no further processsing is required.

If at any time the constraints given through the `TaskInfo` object does not
hold anymore, or if the system deems it necessary, `onStopTask(...)` will be
invoked, requiring all activity to cease immediately. The task can return true
if the task needs to be rescheduled since it was canceled, or false otherwise.

**The system will hold a wakelock from the time `onStartTask(...)` is invoked
until either the task itself invokes the `TaskFinishedCallback`, or
`onStopTask(...)` is invoked.**
