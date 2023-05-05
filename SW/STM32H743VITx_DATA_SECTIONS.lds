SECTIONS
{
  .DTCM :
  {
    . = ALIGN(4);
    *(.DTCM)
    . = ALIGN(4);
  } >DTCMRAM

  .D1 :
  {
    . = ALIGN(4);
    __d1_start__ = .;
    *(.D1)
    . = ALIGN(4);
  } >RAM_D1

  .D2 :
  {
    . = ALIGN(4);
    __d2_start__ = .;
    *(.D2)
    . = ALIGN(4);
  } >RAM_D2

  .D3 :
  {
    . = ALIGN(4);
    __d3_start__ = .;
    *(.D3)
    . = ALIGN(4);
  } >RAM_D3
}
  
