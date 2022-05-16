# Configuration mode

This mode is used to edit or upload profiles with ftp.

Profile 10 is reserved for configuration mode.

In this mode, controller is creating AP and FTP server.

It is possible to change Configuration mode default properties by editing profile10.json.


profile10.json
``
{
    "transmitter_mode" : "CONFIG",
    "ap_name" : "hxrct",
    "ap_password" : "",
    "ftp_user" : "anonymous",
    "ftp_password" : "anonymous"
}

``