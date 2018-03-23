git clone https://github.com/dwrobel/appveyor-client.git
cd appveyor-client
git checkout delete-build-support
python setup.py install --user
cd ..
python ci/appveyor-custodian.py
