#experimental file write script to rewrite a connman settings file

# link config file to connman service
#settings_file_name = "/var/lib/connman/" + selected_network.serviceKey + "/settings"
settings_file_name = "settings"

new_file_name = "settings"

ssid = "SecureWireless"
skey = "wifi_xxx_xxx_managed_psk"


config_file_line = "Config.file=" + ssid.lower() + "\n"
config_ident_line = "Config.ident=service_" + skey + "\n"


settings_file = open(settings_file_name,'r')
settings_old = settings_file.read()
settings_file.close()

settings_file = open(new_file_name,'w')

lines = settings_old.split('\n')

print(lines)

for line in lines:
	if "Config.file" in line:
		settings_file.write(config_file_line)
	elif "Config.ident" in line:
		settings_file.write(config_ident_line)
	elif line:
		settings_file.write(line + "\n")

if "Config.file" not in settings_old:
	settings_file.write(config_file_line)
if "Config.ident" not in settings_old:
	settings_file.write(config_ident_line)

settings_file.close()

# Config.file=<SSIDLOWER>
# Config.ident=service_<SERVICEKEY>