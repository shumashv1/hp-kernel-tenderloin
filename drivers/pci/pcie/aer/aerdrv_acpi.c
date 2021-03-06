/*
 * Access ACPI _OSC method
 *
 * Copyright (C) 2006 Intel Corp.
 *	Tom Long Nguyen (tom.l.nguyen@intel.com)
 *	Zhang Yanmin (yanmin.zhang@intel.com)
 *
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/acpi.h>
#include <linux/pci-acpi.h>
#include <linux/delay.h>
#include <acpi/apei.h>
#include "aerdrv.h"

/**
 * aer_osc_setup - run ACPI _OSC method
 * @pciedev: pcie_device which AER is being enabled on
 *
 * @return: Zero on success. Nonzero otherwise.
 *
 * Invoked when PCIe bus loads AER service driver. To avoid conflict with
 * BIOS AER support requires BIOS to yield AER control to OS native driver.
 **/
int aer_osc_setup(struct pcie_device *pciedev)
{
	acpi_status status = AE_NOT_FOUND;
	struct pci_dev *pdev = pciedev->port;
	acpi_handle handle = NULL;

	if (acpi_pci_disabled)
		return -1;

	handle = acpi_find_root_bridge_handle(pdev);
	if (handle) {
		status = acpi_pci_osc_control_set(handle,
					OSC_PCI_EXPRESS_AER_CONTROL |
					OSC_PCI_EXPRESS_CAP_STRUCTURE_CONTROL);
	}

	if (ACPI_FAILURE(status)) {
		dev_printk(KERN_DEBUG, &pciedev->device, "AER service couldn't "
			   "init device: %s\n",
			   (status == AE_SUPPORT || status == AE_NOT_FOUND) ?
			   "no _OSC support" : "_OSC failed");
		return -1;
	}

	return 0;
}

#ifdef CONFIG_ACPI_APEI
static inline int hest_match_pci(struct acpi_hest_aer_common *p,
				 struct pci_dev *pci)
{
	return	(0           == pci_domain_nr(pci->bus) &&
		 p->bus      == pci->bus->number &&
		 p->device   == PCI_SLOT(pci->devfn) &&
		 p->function == PCI_FUNC(pci->devfn));
}

struct aer_hest_parse_info {
	struct pci_dev *pci_dev;
	int firmware_first;
};

static int aer_hest_parse(struct acpi_hest_header *hest_hdr, void *data)
{
	struct aer_hest_parse_info *info = data;
	struct acpi_hest_aer_common *p;
	u8 pcie_type = 0;
	u8 bridge = 0;
	int ff = 0;

	switch (hest_hdr->type) {
	case ACPI_HEST_TYPE_AER_ROOT_PORT:
		pcie_type = PCI_EXP_TYPE_ROOT_PORT;
		break;
	case ACPI_HEST_TYPE_AER_ENDPOINT:
		pcie_type = PCI_EXP_TYPE_ENDPOINT;
		break;
	case ACPI_HEST_TYPE_AER_BRIDGE:
		if ((info->pci_dev->class >> 16) == PCI_BASE_CLASS_BRIDGE)
			bridge = 1;
		break;
	default:
		return 0;
	}

	p = (struct acpi_hest_aer_common *)(hest_hdr + 1);
	if (p->flags & ACPI_HEST_GLOBAL) {
		if ((info->pci_dev->is_pcie &&
		     info->pci_dev->pcie_type == pcie_type) || bridge)
			ff = !!(p->flags & ACPI_HEST_FIRMWARE_FIRST);
	} else
		if (hest_match_pci(p, info->pci_dev))
			ff = !!(p->flags & ACPI_HEST_FIRMWARE_FIRST);
	info->firmware_first = ff;

	return 0;
}

static void aer_set_firmware_first(struct pci_dev *pci_dev)
{
	int rc;
	struct aer_hest_parse_info info = {
		.pci_dev	= pci_dev,
		.firmware_first	= 0,
	};

	rc = apei_hest_parse(aer_hest_parse, &info);

	if (rc)
		pci_dev->__aer_firmware_first = 0;
	else
		pci_dev->__aer_firmware_first = info.firmware_first;
	pci_dev->__aer_firmware_first_valid = 1;
}

int pcie_aer_get_firmware_first(struct pci_dev *dev)
{
	if (!dev->__aer_firmware_first_valid)
		aer_set_firmware_first(dev);
	return dev->__aer_firmware_first;
}
#endif
