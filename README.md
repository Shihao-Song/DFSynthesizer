# DFSynthesizer

DFSynthesizer is a dataflow-based synthesizer that enables SNN-to-neuromorphic-hardware mapping exploration.

## (Tested) Environment

| Tool | Version |
| ------ | ------ |
| Ubuntu | 20.04 |
| Boost C++ Libraries | 1.71 |
| GCC/G++ | 9.3 |
| GNU Make | 4.2.1 |

## Installation

```sh
cd sdf3
make
cd ..
```


## Tutorials

### Normal execution (no custom tile binding specified)
#### Command
```sh
cd tutorial/normal_execution
../../sdf3/build/release/Linux/bin/sdf3flow-sdf --settings sdf3.opt
```
#### Console output
```console
...
[DFSynthesizer] Original actor-tile mapping. 
[DFSynthesizer] Tile tile_a: mb_encoding motion_compensation 
[DFSynthesizer] Tile tile_b: mb_decoding 
[DFSynthesizer] Tile tile_c: vlc 
[DFSynthesizer] Tile tile_d: motion_estimation
...
```
#### Go back to the repository root
```sh
cd ../..
```

### Custom actor-to-tile binding
#### Command
```sh
cd tutorial/custom_actor_to_tile_binding
../../sdf3/build/release/Linux/bin/sdf3flow-sdf --settings sdf3_tb_2.opt
```
#### Console output
```console
...
[DFSynthesizer] Original actor-tile mapping. 
[DFSynthesizer] Tile tile_0: actor_0 actor_3 
[DFSynthesizer] Tile tile_1: actor_5 actor_7 
[DFSynthesizer] Tile tile_2: actor_2 
[DFSynthesizer] Tile tile_3: actor_4 actor_1
...
[DFSynthesizer] Custom actor-tile mapping.
[DFSynthesizer] Tile tile_0: actor_0 actor_1 actor_2 actor_3 
[DFSynthesizer] Tile tile_1: actor_4 actor_5 actor_7
...
```
#### Investigate sdf3_tb_2.opt
```sh
vim sdf3_tb_2.opt
```
```html
...
9     <TileBinding file="tile_binding_2.txt"/>
...
```
#### Investigate tile_binding_2.txt
```sh
vim tile_binding_2.txt
```
```vim
0 0 1 2 3
1 4 5 7
```
#### tile_binding_2.txt explanation
```txt
0 0 1 2 3
1 4 5 7
    Map 0,1,2,3 to tile 0
    Map 4,5,7 to tile 1
```
