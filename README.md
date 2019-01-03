# orange
for fun
openssl enc -des3 -salt -k $enckey -in tmp.img  -out $encimagename 
openssl des3 -d -salt -k $key -in "$image"  -out "$decimage"							


