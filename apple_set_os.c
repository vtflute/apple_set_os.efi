// Copyright (c) 2015 Bruno Bierbaumer

#include <efibind.h>
#include <efidef.h>
#include <efidevp.h>
#include <eficon.h>
#include <efiapi.h>
#include <efierr.h>
#include <efiui.h>
#include <efiprot.h>
#include <efilib.h>

#define APPLE_SET_OS_VENDOR  "Apple Inc."
#define APPLE_SET_OS_VERSION "Mac OS X 10.9"

static EFI_GUID APPLE_SET_OS_GUID = { 0xc5c5da95, 0x7d5c, 0x45e6, { 0xb2, 0xf1, 0x3f, 0xd5, 0x2b, 0xb1, 0x00, 0x77 }};
static EFI_GUID APPLE_GPU_GUID =    { 0xfa4ce28d, 0xb62f, 0x4c99, { 0x9c, 0xc3, 0x68, 0x15, 0x68, 0x6e, 0x30, 0xf9 }};

typedef struct efi_apple_set_os_interface {
	UINT64 version;
	EFI_STATUS (EFIAPI *set_os_version) (IN CHAR8 *version);
	EFI_STATUS (EFIAPI *set_os_vendor) (IN CHAR8 *vendor);
} efi_apple_set_os_interface;

EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *systemTable)
{
	SIMPLE_TEXT_OUTPUT_INTERFACE *conOut = systemTable->ConOut;
	conOut->OutputString(conOut, L"apple_set_os started\r\n");

	efi_apple_set_os_interface *set_os = NULL;

	EFI_STATUS status  = systemTable->BootServices->LocateProtocol(&APPLE_SET_OS_GUID, NULL, (VOID**) &set_os);
	if(EFI_ERROR(status) || set_os == NULL) {
		conOut->OutputString(conOut, L"Could not locate the apple set os protocol.\r\n");
		return status;
	}

	if(set_os->version != 0){
		status = set_os->set_os_version((CHAR8 *) APPLE_SET_OS_VERSION);
		if(EFI_ERROR(status)){
			conOut->OutputString(conOut, L"Could not set version.\r\n");
			return status;
		}
		conOut->OutputString(conOut, L"Set os version to " APPLE_SET_OS_VERSION  ".\r\n");
	}

	status = set_os->set_os_vendor((CHAR8 *) APPLE_SET_OS_VENDOR);
	if(EFI_ERROR(status)){
		conOut->OutputString(conOut, L"Could not set vendor.\r\n");
		return status;
	}
	conOut->OutputString(conOut, L"Set os vendor to " APPLE_SET_OS_VENDOR  ".\r\n");

	if(systemTable->RuntimeServices->GetVariable != NULL) {
		conOut->OutputString(conOut, L"Try to GetVariable\r\n");
	}

	UINT32 Attributes;
	UINTN DataSize;
	UINT32 ReturnValue;

	status = systemTable->RuntimeServices->GetVariable(L"gpu-power-prefs", &APPLE_GPU_GUID, &Attributes, &DataSize, &ReturnValue);
	if(EFI_ERROR(status)){
		conOut->OutputString(conOut, L"Could not get gpu-power-prefs value\r\n");
		return status;
	} else {
		conOut->OutputString(conOut, L"Got variable\r\n");
		UINT32 index = 8*DataSize;
		while (index-- > 0 ) {
			UINT8 v = ReturnValue & (1 << index);
			if( v != 0 ) {
				conOut->OutputString(conOut, L"1");
			}else {
				conOut->OutputString(conOut, L"0");
			}
		}
		conOut->OutputString(conOut, L"\r\n");
		index = 8 * DataSize;
		while (index-- > 0 ) {
			UINT8 v = Attributes & (1 << index);
			if( v != 0 ) {
				conOut->OutputString(conOut, L"1");
			}else {
				conOut->OutputString(conOut, L"0");
			}
		}
	}

	if((ReturnValue & 0x1) == 0) {
		conOut->OutputString(conOut, L"Should set gpu-power-prefs value as 1 to enable iGPU\r\n");
		ReturnValue |= 0x1;
		status = systemTable->RuntimeServices->SetVariable(L"gpu-power-prefs", &APPLE_GPU_GUID, Attributes, DataSize, &ReturnValue);
		if(EFI_ERROR(status)){
			conOut->OutputString(conOut, L"Could not set gpu-power-prefs value\r\n");
			return status;
		}
	}

	return EFI_SUCCESS;
}
