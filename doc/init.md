Sample init scripts and service configuration for soferoxd
==========================================================

Sample scripts and configuration files for systemd, Upstart and OpenRC
can be found in the contrib/init folder.

    contrib/init/soferoxd.service:    systemd service unit configuration
    contrib/init/soferoxd.openrc:     OpenRC compatible SysV style init script
    contrib/init/soferoxd.openrcconf: OpenRC conf.d file
    contrib/init/soferoxd.conf:       Upstart service configuration file
    contrib/init/soferoxd.init:       CentOS compatible SysV style init script

Service User
---------------------------------

All three Linux startup configurations assume the existence of a "soferox" user
and group.  They must be created before attempting to use these scripts.
The OS X configuration assumes soferoxd will be set up for the current user.

Configuration
---------------------------------

At a bare minimum, soferoxd requires that the rpcpassword setting be set
when running as a daemon.  If the configuration file does not exist or this
setting is not set, soferoxd will shutdown promptly after startup.

This password does not have to be remembered or typed as it is mostly used
as a fixed token that soferoxd and client programs read from the configuration
file, however it is recommended that a strong and secure password be used
as this password is security critical to securing the wallet should the
wallet be enabled.

If soferoxd is run with the "-server" flag (set by default), and no rpcpassword is set,
it will use a special cookie file for authentication. The cookie is generated with random
content when the daemon starts, and deleted when it exits. Read access to this file
controls who can access it through RPC.

By default the cookie is stored in the data directory, but it's location can be overridden
with the option '-rpccookiefile'.

This allows for running soferoxd without having to do any manual configuration.

`conf`, `pid`, and `wallet` accept relative paths which are interpreted as
relative to the data directory. `wallet` *only* supports relative paths.

For an example configuration file that describes the configuration settings,
see `contrib/debian/examples/soferox.conf`.

Paths
---------------------------------

### Linux

All three configurations assume several paths that might need to be adjusted.

Binary:              `/usr/bin/groestcoind`  
Configuration file:  `/etc/soferox/soferox.conf`  
Data directory:      `/var/lib/soferoxd`  
PID file:            `/var/run/soferoxd/soferoxd.pid` (OpenRC and Upstart) or `/var/lib/soferoxd/soferoxd.pid` (systemd)  
Lock file:           `/var/lock/subsys/soferoxd` (CentOS)  

The configuration file, PID directory (if applicable) and data directory
should all be owned by the soferox user and group.  It is advised for security
reasons to make the configuration file and data directory only readable by the
soferox user and group.  Access to soferox-cli and other soferoxd rpc clients
can then be controlled by group membership.

### Mac OS X

Binary:              `/usr/local/bin/soferoxd`  
Configuration file:  `~/Library/Application Support/Soferox/soferox.conf`  
Data directory:      `~/Library/Application Support/Soferox`  
Lock file:           `~/Library/Application Support/Soferox/.lock`  

Installing Service Configuration
-----------------------------------

### systemd

Installing this .service file consists of just copying it to
/usr/lib/systemd/system directory, followed by the command
`systemctl daemon-reload` in order to update running systemd configuration.

To test, run `systemctl start soferoxd` and to enable for system startup run
`systemctl enable soferoxd`

### OpenRC

Rename soferoxd.openrc to soferoxd and drop it in /etc/init.d.  Double
check ownership and permissions and make it executable.  Test it with
`/etc/init.d/soferoxd start` and configure it to run on startup with
`rc-update add soferoxd`

### Upstart (for Debian/Ubuntu based distributions)

Drop soferoxd.conf in /etc/init.  Test by running `service soferoxd start`
it will automatically start on reboot.

NOTE: This script is incompatible with CentOS 5 and Amazon Linux 2014 as they
use old versions of Upstart and do not supply the start-stop-daemon utility.

### CentOS

Copy soferoxd.init to /etc/init.d/soferoxd. Test by running `service soferoxd start`.

Using this script, you can adjust the path and flags to the soferoxd program by
setting the SOFEROXD and FLAGS environment variables in the file
/etc/sysconfig/soferoxd. You can also use the DAEMONOPTS environment variable here.

### Mac OS X

Copy org.soferox.soferoxd.plist into ~/Library/LaunchAgents. Load the launch agent by
running `launchctl load ~/Library/LaunchAgents/org.soferox.soferoxd.plist`.

This Launch Agent will cause soferoxd to start whenever the user logs in.

NOTE: This approach is intended for those wanting to run soferoxd as the current user.
You will need to modify org.soferox.soferoxd.plist if you intend to use it as a
Launch Daemon with a dedicated soferox user.

Auto-respawn
-----------------------------------

Auto respawning is currently only configured for Upstart and systemd.
Reasonable defaults have been chosen but YMMV.
