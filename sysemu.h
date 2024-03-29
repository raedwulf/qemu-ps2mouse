#ifndef SYSEMU_H
#define SYSEMU_H
/* Misc. things related to the system emulator.  */

#include "qemu-common.h"
#include "qemu-option.h"
#include "qemu-queue.h"
#include "qemu-timer.h"
#include "notify.h"

#ifdef _WIN32
#include <windows.h>
#include "qemu-os-win32.h"
#endif

#ifdef CONFIG_POSIX
#include "qemu-os-posix.h"
#endif

/* vl.c */
extern const char *bios_name;

#define QEMU_FILE_TYPE_BIOS   0
#define QEMU_FILE_TYPE_KEYMAP 1
char *qemu_find_file(int type, const char *name);

extern int vm_running;
extern const char *qemu_name;
extern uint8_t qemu_uuid[];
int qemu_uuid_parse(const char *str, uint8_t *uuid);
#define UUID_FMT "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"

typedef struct vm_change_state_entry VMChangeStateEntry;
typedef void VMChangeStateHandler(void *opaque, int running, int reason);

VMChangeStateEntry *qemu_add_vm_change_state_handler(VMChangeStateHandler *cb,
                                                     void *opaque);
void qemu_del_vm_change_state_handler(VMChangeStateEntry *e);

void vm_start(void);
void vm_stop(int reason);

uint64_t ram_bytes_remaining(void);
uint64_t ram_bytes_transferred(void);
uint64_t ram_bytes_total(void);

int64_t cpu_get_ticks(void);
void cpu_enable_ticks(void);
void cpu_disable_ticks(void);

void qemu_system_reset_request(void);
void qemu_system_shutdown_request(void);
void qemu_system_powerdown_request(void);
int qemu_shutdown_requested(void);
int qemu_reset_requested(void);
int qemu_powerdown_requested(void);
extern qemu_irq qemu_system_powerdown;
void qemu_system_reset(void);

void qemu_add_exit_notifier(Notifier *notify);
void qemu_remove_exit_notifier(Notifier *notify);

void do_savevm(Monitor *mon, const QDict *qdict);
int load_vmstate(const char *name);
void do_delvm(Monitor *mon, const QDict *qdict);
void do_info_snapshots(Monitor *mon);

void cpu_synchronize_all_states(void);
void cpu_synchronize_all_post_reset(void);
void cpu_synchronize_all_post_init(void);

void qemu_announce_self(void);

void main_loop_wait(int nonblocking);

int qemu_savevm_state_begin(Monitor *mon, QEMUFile *f, int blk_enable,
                            int shared);
int qemu_savevm_state_iterate(Monitor *mon, QEMUFile *f);
int qemu_savevm_state_complete(Monitor *mon, QEMUFile *f);
void qemu_savevm_state_cancel(Monitor *mon, QEMUFile *f);
int qemu_loadvm_state(QEMUFile *f);

/* SLIRP */
void do_info_slirp(Monitor *mon);

/* OS specific functions */
void os_setup_early_signal_handling(void);
char *os_find_datadir(const char *argv0);
void os_parse_cmd_args(int index, const char *optarg);
void os_pidfile_error(void);

typedef enum DisplayType
{
    DT_DEFAULT,
    DT_CURSES,
    DT_SDL,
    DT_NOGRAPHIC,
} DisplayType;

extern int autostart;
extern int incoming_expected;
extern int bios_size;

typedef enum {
    VGA_NONE, VGA_STD, VGA_CIRRUS, VGA_VMWARE, VGA_XENFB
} VGAInterfaceType;

extern int vga_interface_type;
#define cirrus_vga_enabled (vga_interface_type == VGA_CIRRUS)
#define std_vga_enabled (vga_interface_type == VGA_STD)
#define xenfb_enabled (vga_interface_type == VGA_XENFB)
#define vmsvga_enabled (vga_interface_type == VGA_VMWARE)

extern int graphic_width;
extern int graphic_height;
extern int graphic_depth;
extern uint8_t irq0override;
extern DisplayType display_type;
extern const char *keyboard_layout;
extern int win2k_install_hack;
extern int rtc_td_hack;
extern int alt_grab;
extern int ctrl_grab;
extern int usb_enabled;
extern int smp_cpus;
extern int max_cpus;
extern int cursor_hide;
extern int graphic_rotate;
extern int no_quit;
extern int no_shutdown;
extern int semihosting_enabled;
extern int old_param;
extern int boot_menu;
extern QEMUClock *rtc_clock;

#define MAX_NODES 64
extern int nb_numa_nodes;
extern uint64_t node_mem[MAX_NODES];
extern uint64_t node_cpumask[MAX_NODES];

#define MAX_OPTION_ROMS 16
extern const char *option_rom[MAX_OPTION_ROMS];
extern int nb_option_roms;

#define MAX_PROM_ENVS 128
extern const char *prom_envs[MAX_PROM_ENVS];
extern unsigned int nb_prom_envs;

/* pci-hotplug */
void pci_device_hot_add(Monitor *mon, const QDict *qdict);
void drive_hot_add(Monitor *mon, const QDict *qdict);
void do_pci_device_hot_remove(Monitor *mon, const QDict *qdict);

/* serial ports */

#define MAX_SERIAL_PORTS 4

extern CharDriverState *serial_hds[MAX_SERIAL_PORTS];

/* parallel ports */

#define MAX_PARALLEL_PORTS 3

extern CharDriverState *parallel_hds[MAX_PARALLEL_PORTS];

#define TFR(expr) do { if ((expr) != -1) break; } while (errno == EINTR)

#ifdef HAS_AUDIO
struct soundhw {
    const char *name;
    const char *descr;
    int enabled;
    int isa;
    union {
        int (*init_isa) (qemu_irq *pic);
        int (*init_pci) (PCIBus *bus);
    } init;
};

extern struct soundhw soundhw[];
#endif

void do_usb_add(Monitor *mon, const QDict *qdict);
void do_usb_del(Monitor *mon, const QDict *qdict);
void usb_info(Monitor *mon);

void rtc_change_mon_event(struct tm *tm);

void register_devices(void);

#endif
