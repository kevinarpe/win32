# Passport: The Password Helper

Do you need to regularly input a username and password?

Does your company limit which software that can be installed?

If yes, then this tool will help.  TL;DR: This tool is a poor man's version of `1password`.

You can easily compile this code from MSYS2.

```
# Build the common library
$ common/build.bash --clean --release

# Build the app
$ passport/build.bash --clean --release

# Run the app
$ passport/passport.exe sample_config.txt
```

See `sample_config.txt` to customise the app.

