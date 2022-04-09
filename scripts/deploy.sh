src=$PWD

cd ~ || exit

rm -rf ckchao

heroku login
heroku git:clone -a ckchao

rm -rf ~/ckchao/Aptfile  ~/ckchao/Makefile  ~/ckchao/Procfile  ~/ckchao/frameworks  ~/ckchao/server  ~/ckchao/server.cert  ~/ckchao/server.key

cp -rf "$src"/frameworks ~/ckchao/.
cp -rf "$src"/server ~/ckchao/.
cp -rf "$src"/server.cert ~/ckchao/.
cp -rf "$src"/server.key ~/ckchao/.
cp -rf "$src"/deploy_configs/. ~/ckchao/

cd ckchao || exit

git add .
git add -f *.a
git commit -am "script deploy"
git push heroku master
