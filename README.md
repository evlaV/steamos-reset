# steamos-reset

SteamOS Factory Reset tools (UI and backend)

## Quick Start

Building and installing:

    ```
    autoreconf -ivf # needed first time only
    ./configure --prefix=/usr --libexecdir=/usr/lib --sbindir=/usr/bin
    make
    sudo make install
    ```
By default the qml UI is built. You can choose either or both UIs:

    ```
    ./configure --prefix=/usr --libexecdir=/usr/lib --sbindir=/usr/bin \
        --with-ui=cef
    ```
    ```
    ./configure --prefix=/usr --libexecdir=/usr/lib --sbindir=/usr/bin \
        --with-ui=all
    ```

Adding to your desktop:

   ```
   cd ~/Desktop && \
     ln -s /usr/share/applications/steamos-factory-reset.*.desktop .
   ```

### Skipping the actual OS reset step

The actual fetch-and-install of an OS image can be slow, so if you're
not interested in that you can tell the backend to skip that stage with:

   ```
   sudo touch /run/.skip-os-reset
   ```

It will still spend a few seconds emitting some dummy progress messages
and will set the next boot to the other slot, as if a real update had
happened, but no new OS image will be fetched or installed.

### Runnning the CLI tool:

The CLI tool emits log lines to standard error and a JSON result
on stdout on completion of the requested operation.

  ```
  steamos-reset-tool boot-status
  ```

  ```
  {
    "service": "boot-status",
    "version": "0.02",
    "status": 200,
    "message": "Boot Status",
    "boot": {
      "current": "A",
      "next": "A",
      "reset-list": []
    }
  }

  ```
The boot-status command shows you the current boot configuration and which,
if any, filesystem resets are scheduled for the next boot.

  ```
  steamos-reset-tool os-status
  ```

  ```
  {
    "service": "os-status",
    "version": "0.02",
    "status": 200,
    "message": "OS Status Check Complete",
    "uuid": "7141edd8-59ee-482a-99a6-0c6093b31fdc",
    "update": {
      "needed": 1,
      "url": "https://steamdeck-images.…/…/steamdeck-20221221.2-snapshot.raucb"
    }
  }
  ```

The os-status command fetches a reset OS image candidate URL and caches it.
It also determines whether an OS reset is needed based on the RAUC caibx file
in the current image (if there is one).

Subsequent invocations will not re-fetch the metadata and choose a new candidate
until a reboot happens or the ```cleanup``` command is run.

  ```
  steamos-reset-tool factory-reset
  ```

  ```
  {
    "service": "boot-status",
    "version": "0.02",
    "status": 200,
    "message": "Boot Status",
    "boot": {
      "current": "A",
      "next": "B",
      "reset-list": [
        {
          "device": "/dev/nvme0n1p8",
          "label": "User-data (shared)"
        },
        {
          "device": "/dev/nvme0n1p6",
          "label": "OS-data (A)"
        },
        {
          "device": "/dev/nvme0n1p7",
          "label": "OS-data (B)"
        }
      ]
    }
  }
  ```

The factory-reset command fetches the cached OS image URL and installs it
into the slot that's not currently booted. It then shows you the boot-status.

  ```
  steamos-reset-tool undo-reset
  ```

  ```
  {
    "service": "undo-reset",
    "uuid": "b4932abf-8532-4a21-9d6f-b58f20acfac8",
    "status": 200,
    "message": "Removing factory-reset configuration"
  }
  ```

The undo-reset command removes any scheduled filesystem reset operations.

In addition the following commands are also available, but not needed in normal
reset workflow:

  ```
  steamos-reset-tool status
  steamos-reset-tool status uuid=deadbeef-abad-1dea-1337-d155a715f1ed
  ```

Show the status of all sessions, or just of the specified session

  ```
  steamos-reset-tool cleanup
  ```

Remove all completed sessions (status 200 or no longer have a running process)

  ```
  steamos-reset-tool debug
  ```

Dump some plaintext debug info about the environment in which the reset backends
run. This is the only non-JSON producing command.

## Contents

This repo contains:

 - Web services to provide the underlying functionality
   - Web service scripts 
   - a SUID wrapper for those scripts that need it
   - lighttpd configuration
   - a systemd unit to provide said web services
   - a SUID wrapper to turn the unit on and off

 - A CEF UI to present the factory reset functionality to the user
 - A QML UI for the same
 - A wrapper script that starts and stops the web service
 - .desktop files to launch the two UIs using the wrapper script

## lighttpd Integration

A lighttpd config file called "steamos-repair.conf" wil be generated
and installed in /etc/lighttpd/.

A systemd service (steamos-reset.service) is provided which runs the
web service on port 8080 on the loopback interface.

## Build dependencies

For everything:

  * autoconf
  * automake
  * gcc
  * make
  
For the CEF ui:

  * cmake

For the QML ui

  * qt5

## Install dependencies

  * lighttpd
  * coreutils
  * bash
  * steamos-bootconf

## Transport details

  * Services:
    * will be accessible over HTTP 
    * will be avilable at address 127.0.0.1 (ie on the loopback interface)
    * will be available on port 8080
    * will respond with application/json payloads

### Debugging

NOTE: the wrapper program handles turning the query string into command line
arguments of the form name=value passed via execv(3), so they are not subject
to shell globbing, word splitting, and all that stuff.

This has the added advantage that you can run the commands from the source
tree:

    autoreconf -ivf # needed first time only
    ./configure --prefix=/usr --libexecdir=/usr/lib
    make

    sudo ./os-status
    ./status
    ./status uuid=deadbeef-abad-1dea-1337-d155a715f1ed

NOTE: lighttpd runs with an isolated /tmp, so you can't share results/sessions
between the web service and debug sessions run from a terminal.

NOTE: When run from a terminal, stderr isn't redirected, so you'll see
any stderr output.

## Service response

The response to each service request will be a JSON payload.
It will always contain the following elements:

  * service: (string) the name of the service
  * version: (string) x.y.z style service version
  * status: (integer) the status of the response
    * These will follow the http error codes, so:
      * 200 = success
      * 102 = long running process started
      * 400 and up = some sort of error
    * This is distinct from the HTTP status of the _transport_
  * message: (string) a short description, eg "reset initiated"

In addition if the status is 100-199:

  * uuid: (string) a uuid to identify this request
    * eg: deadbeef-abad-1dea-1337-d155a715f1ed
      * used by long running requestS to identify log messages relating to them

## Services

These are avaiulable at the following URLs

### /os-status?uuid=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

If no uuid is supplied checks the status of the install OS, including:

 - Whether it has been modified or needs to be re-set to a 'vanilla' state
 - Which stable release (if any) is needed to restore it to a vanilla state

It is only possible to check the unmodified state if the RAUC caibx has
been included in the image at a well-known location. If this file is
not present os-status will proceed as if the unmodified check failed.

Currently we cannot install the same image in two different slots, so the
image picked is the most recent stable image that is _not_ the current
image.

sample output:

```
  {"service": "os-status",
   "version": "0.02",
   "status": 102,
   "message": "Checking OS Update Target",
   "uuid":"b1a8b748-c1a0-4252-90a4-05014dedb709"}
```

If a uuid is supplied and that session has completed then the results
of that session are returned in full:

sample outputs:

Still ongoing:

```
  {"service": "os-status",
   "version": "0.02",
   "status": 102,
   "message": "OS status check ongoing",
   "uuid":"b1a8b748-c1a0-4252-90a4-05014dedb709"}
```

Completed, reinstall required:

```
  {"service": "os-status",
   "version": "0.02",
   "status": 200,
   "message": "OS Status Check Complete",
   "uuid":"b1a8b748-c1a0-4252-90a4-05014dedb709",
   "update":
   {"needed":1,
    "url":"https://…/steamdeck/20220817.1/steamdeck-20220817.1-snapshot.raucb"}}
```

Completed, OS is unmodified:

```
  {"service": "os-status",
   "version": "0.02",
   "status": 200,
   "message": "OS Status Check Complete",
   "uuid":"b1a8b748-c1a0-4252-90a4-05014dedb709",
   "update":{"needed":0, "url":""}}
```

Session ID is not from an os-status session:

```
  {"service": "os-status",
   "version": "0.02",
   "status": 400,
   "message": "Session b1a8b748-c1a0-4252-90a4-05014dedb708 is not an os-status check",
   "uuid":"b1a8b748-c1a0-4252-90a4-05014dedb708"}
```

### /boot-status

Returns some information about the boot configuration, including current and 
next boot images and factory-reset actionds (if any) configured to occur on
the next boot:

sample output:

Normal boot status (if image B is the primary):

```
  {"service": "boot-status",
   "version": "0.02",
   "status": 200,
   "message": "Boot Status",
   "boot":
      {"current": "B",
       "next": "B", 
       "reset-list": []}}
```

After an OS reinstall, when a reset has been configured:

```
"service": "boot-status",
 "version": "0.02",
 "status": 200,
 "message": "Boot Status",
 "boot":
    {"current": "B",
     "next": "A", 
     "reset-list": [ {"device":"/dev/nvme0n1p8","label":"User-data (shared)"},
                     {"device":"/dev/nvme0n1p6","label":"OS-data (A)"},
                     {"device":"/dev/nvme0n1p7","label":"OS-data (B)"} ]}}
```

### /status

Returns the current status of all long running sessions:

```
  {"status": 200,
   "version": "0.02",
   "message": "OS Reset Session ",
   "status_list":
   {"1e774b3c-df64-4dcb-b01c-5c256119780b":
     [200, "factory-reset", "Factory reset ready", 0],
    "eae4a572-9535-4a0c-bb4c-08f9746a14db":
     [200, "os-status", "Selecting 20220817.1 (steamdeck)", 0],
    "ff02e764-650b-4f27-9f73-c8c1f2208709":
     [102, "os-status", "Searching for 'steamdeck' image which is not 20221005.1", 173236]}}
```

#### /status?uuid=`UUID`;start=10;max=3

Returns status and log messages for a single request, identified by `UUID`:
If `start` is specified, starts at that log message number.
If `max` is specified, returns no more than that many messages.

```
  {"status": 200,
   "version": "0.02",
   "message": "OS Reset Session ff02e764-650b-4f27-9f73-c8c1f2208709",
   "uuid": "ff02e764-650b-4f27-9f73-c8c1f2208709",
   "status_list": {"ff02e764-650b-4f27-9f73-c8c1f2208709": [200, "os-status", "Selecting 20220817.1 (steamdeck)", 0]},
   "log_messages":["20221118.1000 is a steamdeck-main image",
                   "20221118.100 is a steamdeck-bc image",
                   "20221116.1000 is a steamdeck-main image"]}
```

### /factory-reset?scanuuid=`UUID`

Starts a long-running factory reset session in the background, based on the
previously completed os-status session `UUID`.

This will:
  - Install a fresh OS image selected by the os-status session (if necessary
    - Select the other image for next boot if it was reset.
  - Configure the initrd to reset:
    - /var for image A
    - /var for image B
    - /home (this will also scrub changes to /etc, /srv et al)

sample output:

```
  {"service": "factory-reset",
   "version": "0.02",
   "status": 102,
   "message": "Factory reset started",
   "uuid":"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"} 
```

### /undo-reset

Starts a long-running session that deconfigures any factory reset actions
and sets the boot image back to the current image.

sample output:

```
  {"service": "undo-reset",
   "version": "0.02",
   "status": 102,
   "message": "Undo factory reset started",
   "uuid":"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"} 
```

### /clear?uuid=<UUID>

Erase the data and log messages from session UUID from the cache
