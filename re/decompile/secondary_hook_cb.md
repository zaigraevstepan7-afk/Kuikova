# secondary_hook_cb

- VA: `0x1d81fc`
- insns: 41

## Field offsets (ldr/str immediates)

```
{
  "16": 3,
  "32": 2,
  "1489": 1,
  "688": 1,
  "184": 1,
  "48": 1,
  "80": 1,
  "1264": 1,
  "1496": 1
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
    "from": "0x1d820c",
    "to": "0x1d81bc"
  }
]
```

## Rough pseudocode

```c
// recovered pseudocode: secondary_hook_cb
void secondary_hook_cb(...) {
    call(0x0x1d81bc);  // @0x1d820c
    // branch cbz x19, #0x1d8278
    // branch cbz w8, #0x1d8278
    // branch cbz x8, #0x1d8278
    // branch cbz x8, #0x1d8278
    // branch cbz x8, #0x1d8278
    // branch cbz x0, #0x1d8278
    // branch cbz x0, #0x1d8278
    // branch cbz x1, #0x1d8294
    return;
}
```
