/*
 * x3130_downstream.c
 * TI X3130 pci express downstream port switch
 *
 * Copyright (c) 2010 Isaku Yamahata <yamahata at valinux co jp>
 *                    VA Linux Systems Japan K.K.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "pci_ids.h"
#include "msi.h"
#include "pcie.h"
#include "xio3130_downstream.h"

#define PCI_DEVICE_ID_TI_XIO3130D       0x8233  /* downstream port */
#define XIO3130_REVISION                0x1
#define XIO3130_MSI_OFFSET              0x70
#define XIO3130_MSI_SUPPORTED_FLAGS     PCI_MSI_FLAGS_64BIT
#define XIO3130_MSI_NR_VECTOR           1
#define XIO3130_SSVID_OFFSET            0x80
#define XIO3130_SSVID_SVID              0
#define XIO3130_SSVID_SSID              0
#define XIO3130_EXP_OFFSET              0x90
#define XIO3130_AER_OFFSET              0x100

static void xio3130_downstream_write_config(PCIDevice *d, uint32_t address,
                                         uint32_t val, int len)
{
    pci_bridge_write_config(d, address, val, len);
    pcie_cap_flr_write_config(d, address, val, len);
    pcie_cap_slot_write_config(d, address, val, len);
    msi_write_config(d, address, val, len);
    /* TODO: AER */
}

static void xio3130_downstream_reset(DeviceState *qdev)
{
    PCIDevice *d = DO_UPCAST(PCIDevice, qdev, qdev);
    msi_reset(d);
    pcie_cap_deverr_reset(d);
    pcie_cap_slot_reset(d);
    pcie_cap_ari_reset(d);
    pci_bridge_reset(qdev);
}

static int xio3130_downstream_initfn(PCIDevice *d)
{
    PCIBridge* br = DO_UPCAST(PCIBridge, dev, d);
    PCIEPort *p = DO_UPCAST(PCIEPort, br, br);
    PCIESlot *s = DO_UPCAST(PCIESlot, port, p);
    int rc;

    rc = pci_bridge_initfn(d);
    if (rc < 0) {
        return rc;
    }

    pcie_port_init_reg(d);
    pci_config_set_vendor_id(d->config, PCI_VENDOR_ID_TI);
    pci_config_set_device_id(d->config, PCI_DEVICE_ID_TI_XIO3130D);
    d->config[PCI_REVISION_ID] = XIO3130_REVISION;

    rc = msi_init(d, XIO3130_MSI_OFFSET, XIO3130_MSI_NR_VECTOR,
                  XIO3130_MSI_SUPPORTED_FLAGS & PCI_MSI_FLAGS_64BIT,
                  XIO3130_MSI_SUPPORTED_FLAGS & PCI_MSI_FLAGS_MASKBIT);
    if (rc < 0) {
        return rc;
    }
    rc = pci_bridge_ssvid_init(d, XIO3130_SSVID_OFFSET,
                               XIO3130_SSVID_SVID, XIO3130_SSVID_SSID);
    if (rc < 0) {
        return rc;
    }
    rc = pcie_cap_init(d, XIO3130_EXP_OFFSET, PCI_EXP_TYPE_DOWNSTREAM,
                       p->port);
    if (rc < 0) {
        return rc;
    }
    pcie_cap_flr_init(d);       /* TODO: implement FLR */
    pcie_cap_deverr_init(d);
    pcie_cap_slot_init(d, s->slot);
    pcie_chassis_create(s->chassis);
    rc = pcie_chassis_add_slot(s);
    if (rc < 0) {
        return rc;
    }
    pcie_cap_ari_init(d);
    /* TODO: AER */

    return 0;
}

static int xio3130_downstream_exitfn(PCIDevice *d)
{
    /* TODO: AER */
    msi_uninit(d);
    pcie_cap_exit(d);
    return pci_bridge_exitfn(d);
}

PCIESlot *xio3130_downstream_init(PCIBus *bus, int devfn, bool multifunction,
                                  const char *bus_name, pci_map_irq_fn map_irq,
                                  uint8_t port, uint8_t chassis,
                                  uint16_t slot)
{
    PCIDevice *d;
    PCIBridge *br;
    DeviceState *qdev;

    d = pci_create_multifunction(bus, devfn, multifunction,
                                 "xio3130-downstream");
    if (!d) {
        return NULL;
    }
    br = DO_UPCAST(PCIBridge, dev, d);

    qdev = &br->dev.qdev;
    pci_bridge_map_irq(br, bus_name, map_irq);
    qdev_prop_set_uint8(qdev, "port", port);
    qdev_prop_set_uint8(qdev, "chassis", chassis);
    qdev_prop_set_uint16(qdev, "slot", slot);
    qdev_init_nofail(qdev);

    return DO_UPCAST(PCIESlot, port, DO_UPCAST(PCIEPort, br, br));
}

static const VMStateDescription vmstate_xio3130_downstream = {
    .name = "xio3130-express-downstream-port",
    .version_id = 1,
    .minimum_version_id = 1,
    .minimum_version_id_old = 1,
    .post_load = pcie_cap_slot_post_load,
    .fields = (VMStateField[]) {
        VMSTATE_PCIE_DEVICE(port.br.dev, PCIESlot),
        /* TODO: AER */
        VMSTATE_END_OF_LIST()
    }
};

static PCIDeviceInfo xio3130_downstream_info = {
    .qdev.name = "xio3130-downstream",
    .qdev.desc = "TI X3130 Downstream Port of PCI Express Switch",
    .qdev.size = sizeof(PCIESlot),
    .qdev.reset = xio3130_downstream_reset,
    .qdev.vmsd = &vmstate_xio3130_downstream,

    .is_express = 1,
    .is_bridge = 1,
    .config_write = xio3130_downstream_write_config,
    .init = xio3130_downstream_initfn,
    .exit = xio3130_downstream_exitfn,

    .qdev.props = (Property[]) {
        DEFINE_PROP_UINT8("port", PCIESlot, port.port, 0),
        DEFINE_PROP_UINT8("chassis", PCIESlot, chassis, 0),
        DEFINE_PROP_UINT16("slot", PCIESlot, slot, 0),
        /* TODO: AER */
        DEFINE_PROP_END_OF_LIST(),
    }
};

static void xio3130_downstream_register(void)
{
    pci_qdev_register(&xio3130_downstream_info);
}

device_init(xio3130_downstream_register);

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 *  indent-tab-mode: nil
 * End:
 */
