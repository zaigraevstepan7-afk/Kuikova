# DobbyCodePatch

- VA: `0x23bcf0`
- insns: 52

## Field offsets (ldr/str immediates)

```
{
  "16": 2,
  "32": 2,
  "48": 2,
  "64": 1
}
```

## Strings referenced

```
[]
```

## BL targets

```
[
  {
    "from": "0x23bd10",
    "to": "0x26cc10"
  },
  {
    "from": "0x23bd38",
    "to": "0x26cc20"
  },
  {
    "from": "0x23bd50",
    "to": "0x26c420"
  },
  {
    "from": "0x23bd68",
    "to": "0x26cc20"
  },
  {
    "from": "0x23bd78",
    "to": "0x26c420"
  },
  {
    "from": "0x23bd88",
    "to": "0x26cc20"
  },
  {
    "from": "0x23bd98",
    "to": "0x26cc20"
  },
  {
    "from": "0x23bda4",
    "to": "0x23bdc0"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: DobbyCodePatch
void DobbyCodePatch(...) {
    call(0x0x26cc10);  // @0x23bd10
    call(0x0x26cc20);  // @0x23bd38
    // branch b.ne #0x23bd5c
    call(0x0x26c420);  // @0x23bd50
    // branch b #0x23bd8c
    call(0x0x26cc20);  // @0x23bd68
    call(0x0x26c420);  // @0x23bd78
    call(0x0x26cc20);  // @0x23bd88
    call(0x0x26cc20);  // @0x23bd98
    call(0x0x23bdc0);  // @0x23bda4
    return;
}
```
