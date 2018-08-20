# Auto-Sentry-Lookup
Automated Sentry Testing Notification System  

If you are on probation and your agency uses [http://www.mycallin.com] to allow you to check if you are required to take a drug test, then this program will make your life much easier.  

If not... This program will be entirely useless.  

# Runtime Dependencies
I'm lazy and didn't actually build this will libcurl because ... I'm lazy. So you will need to have `curl` installed on your system.  

If you want to receive email notifications, you will need to have `mail` installed and properly configured to send emails.  

# Installation
Install the repo and run the following:
```sh
chmod +x build.sh
./build.sh
```
If you see this output then you are all set!
```
nigel@mc-east:~/mycallin$ ./build.sh 
sentry successfully compiled!
Be sure to properly configure your mail SMTP server!
```
Otherwise you will need to install the dependencies.  

# First time use
The first time you run the program you need to create your config file.  
If you plan to add a job to your crontab to run this daily (reccomended) then you will either need to make the job cd to the same dir as the program before running it or manually set the config path in the command.  

First take a look at `./sentry --help` to get a little familiar.  

Run this command to generate a new config in the default path:
```
./sentry --no-config --save --add-user email@domain.tld 1234567890 last_name 1234567
```
`1234567890` is the agency phone number.  
`1234567` is your ID number.  

To define a custom path for the config, add this before the email: `--config "~/path/to/config"`

Now if you `cat users.conf` you should see your new configuration file.  
All of the options within `[settings]` are global settings, each user can redefine these options to their own liking and it will only affect that user.  
Other options that are only allowed within user settings are:  

`nickname`: Use this name instead of last_name when notifying.  
`prefix`: Prepend name with this when notifying.  

# Crontab Scheduling
To add a cronjob to run this every day at 5:01am use `crontab -e` to edit your crontab (`sudo` may be necessary on some builds)  

Add this line to the bottom:  
```sh
1 5 * * * cd /path/to/install && ./sentry
```
Or
```sh
1 5 * * * /path/to/sentry --config "/path/to/conf"
```

Follow the same format for adding new users or changing settings.
