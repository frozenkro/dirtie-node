# Network Info

## MQTT

This application will be hard coded to hit a static IP on my home network for the time being. 

In a commercial environment, the dirtie-srv API (and its MQTT Broker sidecar) would be hosted at a public domain. 
In which case, additional security measures would be required, including SSL and per-device Oauth2.

TBD if this will ever come to fruition.

## Provisioning

3-way communication between dirtie-client, dirtie-srv, and dirtie-node.

```flowchart TD
    A[dirtie-client] -->|1 Request Contract| B[dirtie-srv]
    B -->|2 Contract| A
    A -->|3 SSID,PW,Contract| C[dirtie-node]
    C -->|4 Contract| B
```
