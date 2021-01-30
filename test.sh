#!/usr/bin/env bash


#P=/usr/bin/perl
P=perl

hostname

echo "#################### unsigned, valid host name (expect: fail, pass, pass)"
$P ./swaks --to user@host1.nodns.test.swaks.net --from recip@host1.nodns.test.swaks.net --helo hserver \
  --tls --quit tls --tls-verify-ca --tls-verify-target unsigned.example.com --tls-ca-path testing/certs/ca.pem \
  --pipe 'testing/server/smtp-server.pl --silent --domain pipe \
    --cert testing/certs/unsigned.example.com.crt --key testing/certs/unsigned.example.com.key \
    part-0000-connect-standard.txt \
    part-0101-ehlo-all.txt \
    part-0200-starttls-basic.txt \
    part-3000-shutdown-accept.txt \
  ' | grep "=== TLS peer cert"

echo "#################### unsigned and expired (expect: fail, fail, fail)"
$P ./swaks --to user@host1.nodns.test.swaks.net --from recip@host1.nodns.test.swaks.net --helo hserver   --tls --quit tls --tls-ca-path testing/certs/ca.pem   \
    --pipe 'testing/server/smtp-server.pl --silent --domain pipe \
    --cert testing/certs/expired-unsigned.example.com.crt --key testing/certs/expired-unsigned.example.com.key \
    part-0000-connect-standard.txt \
    part-0101-ehlo-all.txt \
    part-0200-starttls-basic.txt \
    part-3000-shutdown-accept.txt \
  ' | grep "=== TLS peer cert"

echo "#################### test 00255 (signed but expired) (expect: pass, fail, fail)"
$P ./swaks --to user@host1.nodns.test.swaks.net --from recip@host1.nodns.test.swaks.net --helo hserver   --tls --quit tls --tls-ca-path testing/certs/ca.pem   \
    --pipe 'testing/server/smtp-server.pl --silent --domain pipe \
    --cert testing/certs/expired-signed.example.com.crt --key testing/certs/expired-signed.example.com.key \
    part-0000-connect-standard.txt \
    part-0101-ehlo-all.txt \
    part-0200-starttls-basic.txt \
    part-3000-shutdown-accept.txt \
  ' | grep "=== TLS peer cert"

echo "##################### test 00263 (signed, target set to --tls-verify-target node.example.com) (expect pass, pass, pass)"
$P ./swaks --to user@host1.nodns.test.swaks.net --from recip@host1.nodns.test.swaks.net --helo hserver \
  --tls --quit tls --tls-verify-ca --tls-verify-target node.example.com --tls-ca-path testing/certs/ca.pem \
  --pipe 'testing/server/smtp-server.pl --silent --domain pipe \
    --cert testing/certs/node.example.com.crt --key testing/certs/node.example.com.key \
    part-0000-connect-standard.txt \
    part-0101-ehlo-all.txt \
    part-0200-starttls-basic.txt \
    part-3000-shutdown-accept.txt \
  ' | grep "=== TLS peer cert"
