FROM registry.oxen.rocks/lokinet-base:latest
RUN /bin/bash -c "git clone https://github.com/majestrate/btpd /usr/local/src/btpd"
RUN /bin/bash -c "cd /usr/local/src/btpd && ./configure && make && make install && mkdir -p /data/btpd && adduser --system --home /data/btpd && cp contrib/btpd-lokinet.service /etc/systemd/system/ && systemctl enable btpd-lokinet.service"
RUN /bin/bash -c "rm -rf /usr/local/src/btpd"
