# Halalium_Bypass

- VA: `0x1d90b8`
- insns: 49

## Field offsets (ldr/str immediates)

```
{
  "16": 2,
  "32": 2,
  "48": 2,
  "24": 1,
  "1776": 1,
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
    "from": "0x1d90e4",
    "to": "0x26c540"
  },
  {
    "from": "0x1d9100",
    "to": "0x26c590"
  },
  {
    "from": "0x1d9140",
    "to": "0x26c560"
  },
  {
    "from": "0x1d9154",
    "to": "0x26c540"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: Halalium_Bypass
void Halalium_Bypass(...) {
    call(0x0x26c540);  // @0x1d90e4
    // branch b.eq #0x1d9108
    call(0x0x26c590);  // @0x1d9100
    // branch b #0x1d90f4
    // branch b.eq #0x1d9164
    call(0x0x26c560);  // @0x1d9140
    call(0x0x26c540);  // @0x1d9154
    // branch b.ne #0x1d9138
    return;
}
```
