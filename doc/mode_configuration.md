# Configuration mode

This mode is used to configure profiles. 

In this mode, controller is creating WIFI access point *("hxrct" without password by default)*, web and FTP servers.

*Profile 10 is reserved for **Configuration mode**. It is not necessary to upload **profile10.json**.* 


# Editing profiles using web file manager

![alt text](https://raw.githubusercontent.com/RomanLut/hx_espnow_rc/main/doc/file_manager.jpg "File manager")

After connecting to AP, go to url http://192.168.4.1.

Use file manager to upload, edit, rename, or delete profiles.


# Exploring filesystem using ftp

After connecting to AP, use ftp server **ftp:://192.168.4.1** to explore filesystem. Default credentials can be found below.

Directories are not supported.

# Changing AP Name, password etc.

It is possible to change **Configuration mode** default properties by uploading **profile10.json**.


**profile10.json** example:
```json
{
    "transmitter_mode" : "CONFIG",
    "ap_name" : "hxrct",
    "ap_password" : "",
    "ftp_user" : "anonymous",
    "ftp_password" : "anonymous"
}

```

As always, you can also upload whole filesystem with PlatformIO.
