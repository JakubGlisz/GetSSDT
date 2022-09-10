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

![Screenshot_2](https://user-images.githubusercontent.com/31781576/189504167-37b1e82b-2052-488a-93df-49dc2922857e.png)

As you may see, this function is referring to the ServiceDescriptorTable.

3. Convert relative address absolute address.


### Implementation
```
PULONGLONG GetSSDT()
{
	ULONGLONG  KiSystemCall64 = __readmsr(0xC0000082);	// Get the address of nt!KeSystemCall64
	ULONGLONG  KiSystemServiceRepeat = 0;
	INT32 Limit = 4096;

	for (int i = 0; i < Limit; i++) {		        // Increase that address until you hit "0x4c/0x8d/0x15"
		if (*(PUINT8)(KiSystemCall64 + i)     == 0x4C
		 && *(PUINT8)(KiSystemCall64 + i + 1) == 0x8D
		 && *(PUINT8)(KiSystemCall64 + i + 2) == 0x15)
		{
			KiSystemServiceRepeat = KiSystemCall64 + i;
			DbgPrint("KiSystemCall64           %p \r\n", KiSystemCall64);
			DbgPrint("KiSystemServiceRepeat    %p \r\n", KiSystemServiceRepeat);

			// Convert relative address to absolute address
			return (PULONGLONG)(*(PINT32)(KiSystemServiceRepeat + 3) + KiSystemServiceRepeat + 7);
		}
	}

	return 0; 
}
```

LSTAR (0xC0000082) - The kernel's RIP SYSCALL entry for 64 bit software.

### Applying structure
```
typedef struct ServiceDescriptorTable {
	UINT32* ServiceTable;
	UINT32* Count;
	UINT32  Limit;
	UINT32* ArgumentTable;
} SDT;
```

```
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) 
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = Unload;

	DbgPrint("\r\nMy driver start:\r\n\r\n");

	SDT *ServiceDescriptorTable = (SDT*)GetSSDT();

	DbgPrint("ServiceDescriptorTable   %p \r\n\r\n", ServiceDescriptorTable);

	DbgPrint("KeServiceDescriptorTable ServiceTable  %p \r\n", ServiceDescriptorTable->ServiceTable);
	DbgPrint("KeServiceDescriptorTable Count         %p \r\n", ServiceDescriptorTable->Count);
	DbgPrint("KeServiceDescriptorTable Limit         %016x \r\n", ServiceDescriptorTable->Limit);
	DbgPrint("KeServiceDescriptorTable ArgumentTable %p \r\n", ServiceDescriptorTable->ArgumentTable);

	return STATUS_SUCCESS;
}
```

Here is the output

![Screenshot_6](https://user-images.githubusercontent.com/31781576/189504341-7260ea69-c3fa-44ac-b70c-8bee10ef9eb8.png)

Checking results with debugger

![Screenshot_7](https://user-images.githubusercontent.com/31781576/189504300-7e599999-fd6d-4706-9535-96202b95faad.png)

### Kernel Virtual Address Shadowing
<p align= 'justify'>
At first when I performed instruction __readmsr(0xc0000082) I got the address of nt!KeSystemCall64Shadow. It happened because the Kernel Virtual Address Shadowing was enabled. This feature was introduced to mitigate the meltdown vulnerability.

You may turn it off by setting the following registers:
</p>


![Screenshot_8](https://user-images.githubusercontent.com/31781576/189504389-d42d38fb-fd6f-4e37-a571-173a4d3e2b36.png)

![Screenshot_5](https://user-images.githubusercontent.com/31781576/189504393-5ce5761c-7142-4e9f-b4f5-f11d41d761d2.png)

_[1]_ https://support.microsoft.com/en-us/topic/kb4072698-windows-server-and-azure-stack-hci-guidance-to-protect-against-silicon-based-microarchitectural-and-speculative-execution-side-channel-vulnerabilities-2f965763-00e2-8f98-b632-0d96f30c8c8e

_[2]_ https://wiki.osdev.org/SYSENTER



