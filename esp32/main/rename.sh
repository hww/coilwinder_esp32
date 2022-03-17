for f in *.hpp; do 
mv -- "$f" "$(basename -- "$f" .hpp).h"
done
