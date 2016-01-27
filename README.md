Data exporter
=================

In the Monroe research project we need to collect data from several different
sources (modem metadata, gps, ...) and write it to different outputs (SQLite,
ZeroMQ, ...). This required us to write a generic and easily extendible tool.

Data exporter is a basic application for collecting and storing data. It is
based around the concept of input and writers, and the application is easy to
extend. Everything is table driven and self-contained, so adding new inputs or
writers is a matter of adding a new entry to the table(s) and implementing the
required callbacks.

An input receives event a source and parses it into a data structure.  All the
different structures generated by inputs have a common header, and the
structures are passed to the writers. The writers then writes the data contained
in the structure to the desired output (for the supported structures).

Currently, we support Netlink as input (from an internal tool) and SQLite and
ZeroMQ (Monroe) as output.  Instead of the earlier mentioned data structure, it
could be more optimal to send for example raw JSON to a writer.  This would be
as easy as adding a new handle callback. The current input/writers is not of
much use outside of Monroe, but should serve as nice examples of how to write
input/writers.

Pull requests are always welcome! One nice feature would be to parallelize the writers. It should be fairly easy, "all" that is needed is a reference counter on each metadata object passed to the writers (and atomic update of this ref. counter).

### Compile instructions

Data exporter is built with CMake and the available writers are selected at
compile time. In order to add SQLite, the following flag must be added to CMake:

    -DSQLITE3=1

ZeroMQ is added with the following flag:

    -DZEROMQ=1

GPSD supports is added with the following flag:

    -DGPSD=1

After that, it is just to run make.

### Command line options

To get the supported command line options for the core application, run:

    ./meta_exporter --help

To see the options for the different inputs/writers, they must be specified. For
example:

   for the core application, run:

    ./meta_exporter --help

To see the options for the different inputs/writers, they must be specified. For
example:

    ./meta_exporter -s -z --help 

At least one input and one writer must be specified. Here is an example of how
to start the application with netlink as input and sqlite as writer:

    ./meta_exporter -s --sql_database ./abcd.db -t --sql_interval 30 --sql_nodeid
    12 --netlink --sql_meta_prefix ./metadata --sql_gps_prefix ./gps --sql_events
    200

### Metadata description

This part is relevant for those using a Monroe experiment node.

#### Interface events

Interface events have event type 0x01 and will be generated by network listener.
We currently only export events for modems. Note that the data, values,
structures, etc. are not stable yet.

Each message, until we have selected one identifier, containes the IMSI, ICCID
and IMEI. No information is exported until these values are available, i.e., PIN
has to be unlocked.

##### Event parameters

A interface event can be one of the following types (stored in event\_param) in
the JSON object:

* REGISTER (1): Modem has been registered/deregistered in network.
* CONNECT (2): Modem has connected/disconnected to network.
* MODE\_CHANGED (3): Mode has changed, for example 2G to LTE.
* SIGNAL\_STRENGTH (4): Signal strength (RSSI, RSRQ, RSRP) has changed.
* LTE\_BAND\_CHANGED (5): LTE band/frequency is changed.
* UPDATE (6): For each modem we send an update message every 30 sec. This update
  contains all the metadata available.

###### Device state

The register and connect messages contain the device state. The values can be
the following:

* UNKNOWN (0): Modem state is unknown.
* REGISTERED (1): Modem is registered to the network.
* UNREGISTERED (2): Modem is unregistered from the network.
* CONNECTED (3): Modem is connected to the network.
* DISCONNECTED (4): Modem is disconnected from network.

###### Device mode/submode

MODE\_CHANGED is export when the modem mode/submode changes. The following
values for mode are possible:

* UNKNOWN (0).
* DISCONNECTED (1).
* NO\_SERVICE (2).
* 2G (3).
* 3G (4).
* LTE (5).

Submode can be one of the following:

* UNKNOWN (0).
* UMTS (1).
* WCDMA (2).
* EVDO (3).
* HSPA (4).
* HSPA+ (5).
* DC HSPA (6).
* DC HSPA+ (7).
* HSDPA (8).
* HSUPA (9).
* HSDPA+HSUPA (10).
* HSDPA+ (11).
* HSDPA+HSUPA (12).
* DC HSDPA+ (13).
* DC HSDPA + HSUPA (14).

###### Other values

All signal strength values (RSSI, RSRQ, RSRP) are in dBm.

The LTE frequence is in mHz.

We read (and store) the operators in three different fiels. `isp_name` is the
literal ISP name read from modem, while `network_mccmnc` is the MCC and MNC read
from the modem. `imsi_mccmnc` is the MCC and MNC read from IMSI.

#### Connection events

Connection events have event type 0x02 and is generated by Dynamic Load
Balancer. This is currently the only metadata that we support.

##### Event parameters

A connection event can be one of the following types (stored in event\_param) in
the JSON object:

* L3\_UP (1): Interface has got IP address.
* L3\_DOWN (2): Interface has lost IP address.
* L4\_UP (3): Interface (address) has internet connectivity.
* L4\_DOWN (4): Interface (address) has lost internet connectivity.
* MODE\_CHANGE (5): Connection mode has changed (2G, 3G, LTE, etc.).
* QUALITY\_CHANGE (6): Quality of connection has changed.
* META\_UPDATE (7): Keep-alive message generated every 30th second.
* META\_MODE\_CHANGE (8): Fake mode change, inserted for two minutes after
  application has crashed in case proper event has been lost. Only inserted if
  mode in update message is different than what is stored in DB.
* META\_QUALITY\_CHANGE (9): Same as fake mode change, but for quality.

###### Interface type

Each message from Dynamic Load Balancer contains the interface type (stored in
interface\_type). The following types are defined:

* UNKNOWN (0).
* MODEM (1).
* PHONE (2).
* USB LAN (3).
* WAN (4).
* WIFI (5).

###### Connection modes

Connection modes can have the following values (stored in event\_value):

* UNKNOWN (1).
* DISCONNECTED (2).
* NO SERVICE (3).
* 2G (4).
* 3G (5).
* LTE (6).

###### Connection qualities

Connection quality can have the following values (stored in event\_value):

* UNKNOWN (0).
* BAD (1).
* GOOD (2).

###### Update message

The update message contains a CSV in the event\_value\_str field. It is
formatted as follows:

w, x, y, z

w is l3 connectivity (0/1), x is l4 connectivity, y is mode and z quality.

#### GPS events

GPS events have type 0x03 and can come from a multitude of sources. The netlink
input expects them to be contained in the variable "nmea\_string" in a JSON
object. It is possible to timestamp the positions by include a timestamp in the
variables "timestamp" in the same object.

