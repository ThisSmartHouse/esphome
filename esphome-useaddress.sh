#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ESPHOME=`which esphome`
YQ=`which yq`

rm -f "$DIR/temp.yaml"
$ESPHOME $1 config > "$DIR/temp.yaml"
pushd $DIR
echo "Uploading using address $2..."
$YQ w -i "$DIR/temp.yaml" wifi.use_address $2
$ESPHOME "$DIR/temp.yaml" compile
$ESPHOME "$DIR/temp.yaml" upload
echo "Cleaing up..."
rm -f "$DIR/temp.yaml"
popd
