# Pull image and upgrade / fetch packages
FROM ubuntu:latest
MAINTAINER Louis Dion-Marcil <ldionmarcil@riseup.net>
RUN apt-get update
RUN apt-get -y upgrade
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install mysql-client mysql-server apache2 php libapache2-mod-php python-setuptools vim-tiny php-mysql php7.0-xml sudo wget iputils-ping nmap cron xvfb phantomjs supervisor


# Hack to get SSL working
RUN mkdir /etc/pki
RUN mkdir /etc/pki/tls
RUN mkdir /etc/pki/tls/certs
RUN wget http://curl.haxx.se/ca/cacert.pem
RUN mv cacert.pem ca-bundle.crt
RUN mv ca-bundle.crt /etc/pki/tls/certs


# Bootstrap MySQL
RUN mkdir /var/run/mysqld
RUN chown mysql:mysql /var/run/mysqld
COPY ./mysql/ /root/mysql/

# Copy and clean httpd workdir
RUN rm -rf /var/www/
COPY ./www /var/www
RUN chown -R root:www-data /var/www/
RUN chmod -R 755 /var/www


# Package sysadmin user
RUN adduser --disabled-password --gecos '' sysadmin
ADD ./files/.bash_history /home/sysadmin/.bash_history
ADD ./files/banking.txt.pgp /home/sysadmin/banking.txt.pgp
RUN chown -R sysadmin:sysadmin /home/sysadmin/
RUN chmod 600 /home/sysadmin/.bash_history
RUN chmod 600 /home/sysadmin/banking.txt.pgp


# Sysadmin crontab
ADD ./files/sysadmin_crontab /var/spool/cron/crontabs/sysadmin
RUN chmod 600 /var/spool/cron/crontabs/sysadmin
RUN chown sysadmin:crontab /var/spool/cron/crontabs/sysadmin
ADD ./scripts/deletetmp.sh /home/sysadmin/deletetmp.sh
RUN chown sysadmin:sysadmin /home/sysadmin/deletetmp.sh
RUN chmod 744 /home/sysadmin/deletetmp.sh


# Setup XSS bot
ADD ./scripts/xss.js /root/xss.js
RUN chmod 700 /root/xss.js
ADD ./files/xss_crontab /var/spool/cron/crontabs/root
RUN chmod 600 /var/spool/cron/crontabs/root
RUN chown root:crontab /var/spool/cron/crontabs/root


# Setup startup scripts
ADD ./configs/000-default.conf /etc/apache2/sites-available/000-default.conf
ADD ./configs/supervisord.conf /etc/supervisor/supervisord.conf
ADD ./scripts/start.sh /root/start.sh
ADD ./scripts/setupmysql.sh /root/setupmysql.sh
ADD ./scripts/httpd.sh /root/httpd.sh
RUN chmod 700 /root/start.sh
RUN chmod 700 /root/setupmysql.sh
RUN chmod 700 /root/httpd.sh

#HACK PLZ WORK
ADD ./files/mysqld.cnf /etc/mysql/mysql.conf.d/mysqld.cnf


# Start main execution loop
EXPOSE 80
#CMD ["/bin/bash", "/root/start.sh"]
ENTRYPOINT ["/root/start.sh"]