#include "ntddk.h"

typedef struct ServiceDescriptorTable {
	UINT32* ServiceTable;
	UINT32* Count;
	UINT32  Limit;
	UINT32* ArgumentTable;
} SDT;

PULONGLONG GetSSDT()
{
	ULONGLONG  KiSystemCall64 = __readmsr(0xC0000082);		// Get the address of nt!KeSystemCall64
	ULONGLONG  KiSystemServiceRepeat = 0;
	INT32 Limit = 4096;

	for (int i = 0; i < Limit; i++) {						// Increase that address until you hit "0x4c/0x8d/0x15"
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

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	DbgPrint("\r\nDriver unload\r\n");
}

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
