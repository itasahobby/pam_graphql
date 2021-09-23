# GraphQL PAM
PAM module to authenticate through GraphQL API

## Usage

Install the module:
```
apt install libpam0g-dev
git clone https://github.com/itasahobby/pam_graphql.git
make install
```

Configure SSH to use the PAM module adding the following to `/etc/pam.d/sshd`:
```
auth       optional     pam_graphql.so url=http://127.0.0.1:9000 method=POST query={query:"query{login(username:\"%PLACEHOLDER1%\",password:\"%PLACEHOLDER2%\")}"}
```

## References
* https://web.archive.org/web/20190523222819/https://fedetask.com/write-linux-pam-module/
* https://ben.akrin.com/?p=1068
