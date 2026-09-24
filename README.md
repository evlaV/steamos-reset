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

The CLI tool emits log lines to stdout.

  ```
  steamos-reset-tool factory-reset
  ```

  ```
  Factory-Reset started at 2026-09-10 16:21:04 +0100
  ID: 20260716.1 - version: 3.8.16 - branch: stable - variant: steamdeck
  New OS image: 0.00%
  atomupd-manager[D]: Debug output enabled
  atomupd-manager[D]: DEBUG:client.py:740: Parsing config from file: /etc/steamos-atomupd/client.conf
  atomupd-manager[D]: DEBUG:client.py:545: The attempts log is missing, assuming no previous failed update attempts
  atomupd-manager[D]: DEBUG:client.py:605: The active slot seed index is located in: /var/lib/steamos-atomupd/rootfs.caibx
  atomupd-manager[D]: DEBUG:client.py:777: Installing an update from the given URL
  atomupd-manager[D]: DEBUG:client.py:510: Getting the rootfs device by parsing the RAUC status
  atomupd-manager[D]: DEBUG:client.py:368: Remounting /tmp with max memory and inodes number
  atomupd-manager[D]: DEBUG:client.py:386: Installing the bundle
  New OS image: 2.08%
  New OS image: 5.00%
  New OS image: 11.11% 06s
  New OS image: 23.97% 03s
  New OS image: 37.28% 01s
  New OS image: 50.29% 01s
  New OS image: 65.46% 00s
  New OS image: 77.45%
  New OS image: 82.13%
  New OS image: 96.16%
  New OS image: 100.00%

  Update completed
  ```

The factory-reset command fetches the cached OS image URL and installs it
into the slot that's not currently booted.

## Contents

This repo contains:

 - Web services to provide the underlying functionality
   - Web service scripts 
   - a SUID wrapper for those scripts that need it

These scripts are now solely used from the CLI interface.

## Build dependencies

For everything:

  * autoconf
  * automake
  * gcc
  * make
  
## Install dependencies

  * coreutils
  * bash
  * steamos-bootconf


### Debugging

NOTE: the wrapper program handles turning the query string into command line
arguments of the form name=value passed via execv(3), so they are not subject
to shell globbing, word splitting, and all that stuff.

This has the added advantage that you can run the commands from the source
tree:

    autoreconf -ivf # needed first time only
    ./configure --prefix=/usr --libexecdir=/usr/lib
    make

    sudo ./factory-reset --reset-os
    ./status
    ./status uuid=deadbeef-abad-1dea-1337-d155a715f1ed

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

These are available at the following URLs

### /boot-status

Returns some information about the boot configuration, including current and 
next boot images and factory-reset actionds (if any) configured to occur on
the next boot:

sample output:

Normal boot status (if image B is the primary):

```
  {"service": "boot-status",
   "version": "0.03",
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
 "version": "0.03",
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
 "version": "0.03",
 "message": "OS Reset Session ",
 "status_list":
{"66d11140-32b3-4688-8d5d-fda9466a124e": [200, "factory-reset", "Update completed", 0, 15]
}}
```

#### /status?uuid=`UUID`;start=10;max=3

Returns status and log messages for a single request, identified by `UUID`:
If `start` is specified, starts at that log message number.
If `max` is specified, returns no more than that many messages.

```
{"status": 200,
 "version": "0.03",
 "message": "OS Reset Session 66d11140-32b3-4688-8d5d-fda9466a124e",
 "uuid": "66d11140-32b3-4688-8d5d-fda9466a124e",
 "status_list": {"66d11140-32b3-4688-8d5d-fda9466a124e": [200, "factory-reset", "Update completed", 0, 15]},
 "log_messages":[
 "New OS image: 80.16%"
,"New OS image: 95.09%"
,"New OS image: 100.00%"
]}
```

### /factory-reset

Starts a long-running factory reset session in the background.

This will:
  - Install a fresh OS image
    - Select the other image for next boot if it was reset
  - Configure the initrd to reset:
    - /var for image A
    - /var for image B
    - /home (this will also scrub changes to /etc, /srv et al)

sample output:

```
  {"service": "factory-reset",
   "version": "0.03",
   "status": 102,
   "message": "Factory reset started",
   "uuid":"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"}
```

### /clear?uuid=<UUID>

Erase the data and log messages from session UUID from the cache
