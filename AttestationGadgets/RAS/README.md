This RAS is deployed on master/control node.

# Remote Attestation Service

- To end user
- To LA verifiers

## Functionalities

- Key management

## Common Interfaces 

A set of remote attestation msg exchange & key exchange interfaces

- `ra_related_functions`
- `DHKE`

## End User Interfaces

- listen: socket `eu` for *end user*

### Data to receive
- ~~`recv_file()`: receive `g_pf` from end user~~
- `recv_pf_info()`, **protected**: key & filename(path) of the `g_pf`

#### TBD
- `recv_command()`: receive command & args

## LA Verifier Interfaces

- listen: socket `lav`for *local attestation verifier* 
- maintain a table of active working nodes and their attestation data of each server

- `send_pf_info()`: send the key & path to the `g_pf` used by the user

### Conrtol function

- TBD: management of LAV/LAC(working node)