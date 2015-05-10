<p align='center'><a href='http://bit.ly/oYBQMe'><img src='http://dl.dropbox.com/u/3968285/banner_ostinato.png' alt='Ostinato - An open-source Packet Crafter/Traffic Generator' /></a></p>
bindconfig is a command line tool to view, enable and disable bindings of windows network components using the INetCfg API.

It is similar to the 'bindview' sample application that is shipped as part of the Windows WDK/DDK but is command line based instead of GUI based and supports only a subset of what bindview supports.

bindconfig was written as part of [ostinato](http://code.google.com/p/ostinato) but is actually a standalone application and maintained separately

# Building #
To build bindconfig, you will need the [Windows Driver Kit](http://www.microsoft.com/whdc/devtools/WDK/default.mspx). After installing the WDK, launch the appropriate Build Environment and type 'build'. The executable is generated in `exe/<build_env_specific_dir>/<platform_specific_dir>`

# Usage #
```
bindconfig.exe [class dev|client|proto|service] [comp <id>] [path <pathToken>] [enable|disable]
```

# Examples #
To view the bindings for all network adaptors (i.e. 'device' class components) type -
```
bindconfig
```
Note: `class dev` is the default

To view the bindings for all protocols type -
```
bindconfig class proto
```

To view the bindings for the TCP/IP protocol, type -
```
bindconfig class proto comp Tcpip
```

To disable all bindings for the Loopback network adaptor (assuming its component id is 7BA478D0-D4E6-4164-BACD-40B3C3CC7CF4), type -

```
bindconfig comp 7BA478D0-D4E6-4164-BACD-40B3C3CC7CF4 disable
```
Note: You can find the component id by using -
```
bindconfig class <class>
```



