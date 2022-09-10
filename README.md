# Finding the Service Descriptor Table address on Windows 10 64-bit


### Background
<p align= 'justify'>
I was reading about SSDT hooks and wanted to implement it on my own. The first task I faced was getting the address of KeServiceDescriptorTable. In the resources that I used it seemed straightforward, but when I tried to make it on my own many challenges occurred along the way. To save you some hustle, I'll try to cover them and provide the solutions I found.
</p>

### 32 - bit
<p align= 'justify'>
On 32-bit, KeServiceDescriptorTable was exported by NTOSKRNL. As far as I know you could just define its structure, import applying it and voilà you had it.
</p>

### 64 - bit

On 64-bit however, it is not exported anymore. The only trick I know to go around it goes like this:

1. Get the address of nt!KeSystemCall64.
2. Increase that address until you hit a specific bytes sequence "0x4c/0x8d/0x15" - hitting it means that you are in the nt!KiSystemServiceRepeat which has the following definition.

As you may see, this function is referring to the ServiceDescriptorTable.

3. Convert relative address absolute address.


### Implementation

LSTAR (0xC0000082) - The kernel's RIP SYSCALL entry for 64 bit software.

### Applying structure


Here is the output

Checking results with debugger

### Kernel Virtual Address Shadowing
<p align= 'justify'>
At first when I performed instruction __readmsr(0xc0000082) I got the address of nt!KeSystemCall64Shadow. It is because the Kernel Virtual Address Shadowing was enabled. This feature was introduced to mitigate the meltdown vulnerability.

You may turn it off by setting the following registers:
</p>


_[1] Disable/enable KVAS msdn page_
https://support.microsoft.com/en-us/topic/kb4072698-windows-server-and-azure-stack-hci-guidance-to-protect-against-silicon-based-microarchitectural-and-speculative-execution-side-channel-vulnerabilities-2f965763-00e2-8f98-b632-0d96f30c8c8e

_[2] MDR_
https://wiki.osdev.org/SYSENTER



