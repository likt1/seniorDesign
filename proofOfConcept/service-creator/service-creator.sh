#!/bin/sh
# based off of gist.github.com/naholyr/4275302

SERVICE_FILE=$(tempfile)
TEMPLATE_FILE=template-service.sh


echo "---validate template---"
if [ -f "$TEMPLATE_FILE" ]
then
	echo "found, continue with service creation..."
else
	echo "template file not found, stopping service creation..."
	exit 1
fi

echo "--- Customize new service ---"
echo "You will now be prompted for some customizations for your new service"
echo "Press Ctrl+C anytime to abort."
echo "Empty values are not accepted."
echo ""

prompt_token() {
	local VAL=""
	while [ "$VAL" = "" ]; do
		echo -n "${2:-$1} : "
		read VAL
		if [ "$VAL" = "" ]; then
			echo "Please provide a value"
		fi
	done
	VAL=$(printf '%q' "$VAL")
	eval $1=$VAL
	sed -i "s/<$1>/$(printf '%q' "$VAL")/g $SERVICE_FILE"
}

prompt_token 'NAME'	'Service name'
if [ -f "/etc/init.d/$NAME" ]; then
	echo "Error: service '$NAME' already exists"
	exit 1
fi

prompt_token 'DESCRIPTION'	' Description'
prompt_token 'COMMAND'		'     Command'
prompt_token 'USERNAME'		'        User'
if ! id -u '$USERNAME' &> /dev/null; then
	echo "Error: user '$USERNAME' not found"
	exit 1
fi

echo ""

echo "--- Installation ---"
if [ ! -w /etc/init.d ]; then
	echo "I don't have nough permissions to install the service..."
	echo "to install it manually run the following:"
	echo ""
	echo "	mv\"$SERVICE_FILE\" \"/etc/init.d/$NAME\""
	echo "	touch \"/var/log/$NAME.log\" && chown \"$USERNAME\" \"/var/log/$NAME.log\""
	echo "	update-rc.d \"$NAME\" defaults"
	echo "	service \"$NAME\" start"
else
	echo "1. mv\"$SERVICE_FILE\" \"/etc/init.d/$NAME\""
	mv -v "$SERVICE_FILE" "/etc/init.d/$NAME"
	echo "2. touch \"/var/log/$NAME.log\" && chown \"$USERNAME\" \"/var/log/$NAME.log\""
	touch "/var/log/$NAME.log" && chown "$USERNAME" "/var/log/$NAME.log"
	echo "3. update-rc.d \"$NAME\" defaults"
	update-rc.d "$NAME" defaults
	echo "4. service \"$NAME\" start"
	service "$NAME" start
fi

echo ""
echo "---Uninstall Instructions---"
echo "The service can uninstall itself:"
echo "	service \"$NAME\" uninstall"
echo "It will simply run update-rc.d -f \"$NAME\" remove && rm -f \"/etc/init.d/$NAME\""
echo ""
echo "--- Done ---"
