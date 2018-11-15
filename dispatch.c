#include <gtk/gtk.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

#define SYS_HEIGHT 0
#define TOOL_WIDTH 200
#define TOOL_HEIGHT_HIDE 2
#define TOOL_HEIGHT_SHOW 40

#define battery_fd_capacity "/root/terminal/capacity"
#define battery_fd_status   "/root/terminal/status"
#define net_fd_status       "/root/terminal/net_status"
#define net_fd_signal      "/root/terminal/wireless_signal"

enum {
    BATTERY_0,
    BATTERY_MISSING,
    BATTERY_EMPTY,
    BATTERY_CAUTION,
    BATTERY_CAUTION_CHARGE,
    BATTERY_LOW,
    BATTERY_LOW_CHARGE,
    BATTERY_GOOD,
    BATTERY_GOOD_CHARGE,
    BATTERY_FULL,
    BATTERY_FULL_CHARGE,
};

enum {
    NETWORK_0,
    NETWORK_ERROR,
    NETWORK_OFFLINE,
    NETWORK_NORMAL,
    NETWORK_WIRELESS_EXCE,
    NETWORK_WIRELESS_GOOD,
    NETWORK_WIRELESS_OK,
    NETWORK_WIRELESS_WEAK,
    NETWORK_WIRELESS_NONE,
    NETWORK_WIRELESS_OFFLINE,
};

GtkWidget *window;
GtkWidget *window_tool;
GtkWidget *button_battery;
GtkWidget *button_network;

GdkScreen* screen;
gint width_screen, height_screen;
pid_t pid_remote_viewer[16];

void cb_show();
void cb_hide();
void cb_quit();

void add_icon_theme()
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wired-no-route-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wired-no-route-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wired-offline-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wired-offline-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wired-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wired-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wireless-signal-excellent-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wireless-signal-excellent-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wireless-signal-good-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wireless-signal-good-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wireless-signal-ok-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wireless-signal-ok-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wireless-signal-weak-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wireless-signal-weak-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wireless-signal-none-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wireless-signal-none-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/network-wireless-offline-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("network-wireless-offline-rviewer",NULL,pixbuf);

    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-missing-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-missing-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-empty-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-empty-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-empty-charging-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-empty-charging-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-caution-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-caution-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-caution-charging-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-caution-charging-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-low-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-low-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-low-charging-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-low-charging-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-good-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-good-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-good-charging-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-good-charging-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-full-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-full-rviewer",NULL,pixbuf);
    pixbuf = gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/24x24/devices/battery-full-charging-rviewer.png",NULL);
    gtk_icon_theme_add_builtin_icon("battery-full-charging-rviewer",NULL,pixbuf);
}

void init_toolbar()
{
    add_icon_theme();
    GtkWidget *hbox = gtk_hbox_new(TRUE,5);

    // network
    GtkWidget *button = GTK_WIDGET(gtk_tool_button_new(NULL,NULL));
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button), "network-idle");
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(button), ("Network Check"));
    gtk_widget_set_sensitive(button, FALSE);
    gtk_container_add(GTK_CONTAINER(hbox), button);
    button_network = button;

    // battery
    button = GTK_WIDGET(gtk_tool_button_new(NULL, NULL));
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button), "battery-full-charged");
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(button), ("Battery Display"));
    gtk_widget_set_sensitive(button, FALSE);
    gtk_container_add(GTK_CONTAINER(hbox), button);
    button_battery = button;

    // exit
    button = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE));
    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(button), ("Disconnect"));
    gtk_widget_show(GTK_WIDGET(button));
    gtk_container_add(GTK_CONTAINER(hbox), button);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_quit), NULL);

    gtk_container_add(GTK_CONTAINER(window_tool), hbox);
    gtk_widget_show_all(window_tool);
}

gint get_battery_capacity()
{
    int capacity_fd;
    int status_fd;
    int battery_cap;
    unsigned char battery_status;
    capacity_fd = open(battery_fd_capacity,O_RDONLY);
    status_fd = open(battery_fd_status,O_RDONLY);
    lseek(capacity_fd,0,SEEK_SET);
    lseek(status_fd,0,SEEK_SET);
    read(capacity_fd, &battery_cap,sizeof(battery_cap));
    read(status_fd, &battery_status,sizeof(battery_status));
    close(capacity_fd);
    close(status_fd);
    printf("battery_status = 0x%x\n",battery_status);
    printf("battery_cap = %d\n",battery_cap);

    if(!battery_cap)
        return BATTERY_MISSING;
    else if (battery_cap == 0)
        return BATTERY_EMPTY;
    else if (battery_cap<10 && battery_cap>0 && battery_status == 0x55)
        return BATTERY_CAUTION;
    else if (battery_cap<10 && battery_cap>0 && battery_status == 0xAA)
        return BATTERY_CAUTION_CHARGE;
    else if (battery_cap<30 && battery_cap>=10 && battery_status == 0x55)
        return BATTERY_LOW;
    else if (battery_cap<30 && battery_cap>=10 && battery_status == 0xAA)
        return BATTERY_LOW_CHARGE;
    else if (battery_cap<90 && battery_cap>=30 && battery_status == 0x55)
        return BATTERY_GOOD;
    else if (battery_cap<90 && battery_cap>=30 && battery_status == 0xAA)
        return BATTERY_GOOD_CHARGE;
    else if (battery_cap<100 && battery_cap>=90 && battery_status == 0x55)
        return BATTERY_FULL;
    else if (battery_cap<100 && battery_cap>=90 && battery_status == 0xAA)
        return BATTERY_FULL_CHARGE;
    else
        return BATTERY_0;
}

gint get_network_type()
{
    unsigned char g_signal_dbm[5];
    int signal_dbm;
    unsigned char net_status;
    int net_fd;
    int signal_fd;
    net_fd = open(net_fd_status,O_RDONLY);
    if(!net_fd)
    {
        printf("Can't acquire net_fd!!\n");
        return -1;
    }
    signal_fd = open(net_fd_signal,O_RDONLY);
    if(!signal_fd)
    {
        printf("Can't acquire net_fd_signal!!\n");
        return -1;
    }
    lseek(net_fd,0,SEEK_SET);
    lseek(signal_fd,0,SEEK_SET);
    read(net_fd,&net_status,sizeof(net_status));
    printf("net_status = 0x%x\n",net_status);
    close(net_fd);
    if(net_status == 0x55)
    {
       return  NETWORK_NORMAL;
    }
    else if (net_status == 0x00)
    {
       return NETWORK_OFFLINE;
    }
    else if (net_status == 0xaa)
    {
       read(signal_fd,g_signal_dbm,5);
       printf("g_signal_dbm = %s\n",g_signal_dbm);
       close(signal_fd);
       g_signal_dbm[3] = '\0';
       signal_dbm = atoi(g_signal_dbm+1);
       signal_dbm = -signal_dbm;
       printf("signal_dbm = %d\n",signal_dbm);
       if(signal_dbm>15 && signal_dbm <30)
          return NETWORK_WIRELESS_EXCE;
       else if (signal_dbm>=30 && signal_dbm<45)
          return NETWORK_WIRELESS_GOOD;
       else if (signal_dbm>=45 && signal_dbm<60)
          return NETWORK_WIRELESS_OK;
       else if (signal_dbm>=60 && signal_dbm<75)
          return NETWORK_WIRELESS_WEAK;
       else if (signal_dbm>=75 && signal_dbm<90)
          return NETWORK_WIRELESS_NONE;
       else
          return NETWORK_WIRELESS_OFFLINE;
    }
    return NETWORK_OFFLINE;
}

void update_toolbar_battery()
{
    gint battery_status;
    battery_status = get_battery_capacity();
    switch(battery_status)
    {
        case BATTERY_MISSING:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-missing-rviewer");
             break;
        case BATTERY_EMPTY:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-empty-rviewer");
             break;
        case BATTERY_CAUTION:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-caution-rviewer");
             break;
        case BATTERY_CAUTION_CHARGE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-caution-charging-rviewer");
             break;
        case BATTERY_LOW:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-low-rviewer");
             break;
        case BATTERY_LOW_CHARGE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-low-charging-rviewer");
             break;
        case BATTERY_GOOD:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-good-rviewer");
             break;
        case BATTERY_GOOD_CHARGE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-good-charging-rviewer");
             break;
        case BATTERY_FULL:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-full-rviewer");
             break;
             break;
        case BATTERY_FULL_CHARGE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_battery),
                                           "battery-full-charging-rviewer");
             break;
        default:
             printf("Battery status is unnormal\n");
             break;
    }
}

void update_toolbar_network()
{
    gint network_status;
    network_status = get_network_type();
    printf("network_status:%d\n", network_status);
    switch(network_status)
    {
        case NETWORK_ERROR:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wired-no-route-rviewer");
             break;
        case NETWORK_OFFLINE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wired-offline-rviewer");
             break;
        case NETWORK_NORMAL:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wired-rviewer");
             break;
        case NETWORK_WIRELESS_EXCE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wireless-signal-excellent-rviewer");
             break;
        case NETWORK_WIRELESS_GOOD:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wireless-signal-good-rviewer");
             break;
        case NETWORK_WIRELESS_OK:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wireless-signal-ok-rviewer");
             break;
        case NETWORK_WIRELESS_WEAK:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wireless-signal-weak-rviewer");
             break;
        case NETWORK_WIRELESS_NONE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wireless-signal-none-rviewer");
             break;
        case NETWORK_WIRELESS_OFFLINE:
             gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button_network),
                                           "network-wireless-offline-rviewer");
             break;
        default:
             printf("Network status is unnormal");
             break;
    }
}

void cb_show()
{
    gtk_widget_hide(window);
    update_toolbar_battery();
    update_toolbar_network();
    gtk_widget_show(window_tool);
}

void cb_hide()
{
    gtk_widget_hide(window_tool);
    gtk_widget_show(window);
}

void cb_quit()
{
    gtk_main_quit();
}

void cb_child_exit(int sig)
{
    pid_t pid;
    int i, stat;
    pid = wait(&stat);
    printf( "remote_viewer pid %d exit.\n", pid);
    for(i=0;i<16;i++)
    {
        if(pid_remote_viewer[i] == pid)
        {
            pid_remote_viewer[i]=0;
            break;
        }
    }
    for(i=0;i<16;i++)
    {
        if(pid_remote_viewer[i] >0)
            break;
    }
    if(i==16)
    {
        printf("dispatch exit.\n");
        exit(0);
    }
}

void exec_system(const char *cmd, char **argv)
{
    pid_t pid;
    int i;

    signal(SIGCHLD,cb_child_exit);
    if((pid = fork())<0)
    {
         printf("fork remote viewer failed.\n");
    }
    else if(pid == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        execvp(cmd, argv);
    }
    else
    {
        for(i=0;i<16;i++)
        {
            if(pid_remote_viewer[i] == 0)
            {
                pid_remote_viewer[i]= pid;
                break;
            }
        }
    }
}

void splitstr(int *n, char *src)
{
    int i=1;
    char *p1=NULL, *p2=src;
    while(1)
    {
        p1 = strstr(p2, " --");
        if(p1)
        {
            if(i==1) n[i]=n[i-1]+p1-p2;
            else n[i]=n[i-1]+p1-p2+3;
            i++;
            p2=p1+3;
        }
        else
        {
            break;
        }
    }
}

void remote_viewer(gchar **args)
{
    int i, j, k, num, line;
    char *argv[16];

    if (args)
    {
        num = g_strv_length(args);
        line = (num + 1)/2;
        for(i =0; i< 16; i++)
        {
            argv[i] = malloc(128);
            memset(argv[i], 0, 128);
        }
        for(i =0; i< g_strv_length(args); i++)
        {
                k=1;
                int n[16] = {0};
                splitstr(n, args[i]);
                printf("args:%s\n",args[i]);
                strcpy(argv[0], "remote-viewer");
                sprintf(argv[1], "--x=%d",(i%2)*width_screen/2);
                sprintf(argv[2], "--y=%d",SYS_HEIGHT+((i+2)/2-1)*height_screen/line);
                sprintf(argv[3], "--width=%d",width_screen/2);
                sprintf(argv[4], "--height=%d",(height_screen-SYS_HEIGHT)/line);

                strncpy(argv[5], args[i], n[1]-n[0]);
                for(j=6; j<g_strv_length(argv); j++)
                {
                   if(n[k] >0)
                   {
                       if(n[k+1]>0)
                           strncpy(argv[j], args[i]+n[k]+1, n[k+1]-n[k]);
                       else
                           strcpy(argv[j], args[i]+n[k]+1);
                   }
                   else
                   {
                       break;
                   }
                   k++;
                }
                argv[j] = NULL;

                exec_system("remote-viewer", argv);
                int jj;
                for(jj =0; jj< 16; jj++)
                {
                    if(argv[jj]) printf("==:%s\n",argv[jj]);
                }

        }
        for(i =0; i< 16; i++)
        {
            free(argv[i]);
        }
    }
}

int main(int argc,char *argv[])
{
    gchar **args = NULL;
    GOptionContext *context;
    GError *error = NULL;

    const GOptionEntry options [] =
    {
        { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &args,
          NULL, "URI|VV-FILE" },
        { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
    };
    context = g_option_context_new (NULL);
    g_option_context_add_main_entries(context, options, NULL);  
    g_option_context_parse (context, &argc, &argv, &error);

    gtk_init(&argc,&argv);

    screen = gdk_screen_get_default();
    width_screen = gdk_screen_get_width(screen);
    height_screen = gdk_screen_get_height(screen);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    window_tool = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_move((GtkWindow *)window, width_screen/2-TOOL_WIDTH/2, 0);
    gtk_window_move((GtkWindow *)window_tool, width_screen/2-TOOL_WIDTH/2, 0);
    gtk_widget_set_size_request(window,TOOL_WIDTH,TOOL_HEIGHT_HIDE);
    gtk_widget_set_size_request(window_tool,TOOL_WIDTH,TOOL_HEIGHT_SHOW);
    gtk_window_set_decorated((GtkWindow *)window,FALSE);
    gtk_window_set_decorated((GtkWindow *)window_tool,FALSE);

    init_toolbar();
    gtk_widget_hide(window_tool);
    gtk_widget_show(window);

    g_signal_connect(window,"destroy",G_CALLBACK(cb_quit),NULL);
    gtk_widget_add_events(window,GDK_ENTER_NOTIFY_MASK);
    g_signal_connect(window,"enter_notify_event",G_CALLBACK(cb_show),NULL);
    gtk_widget_add_events(window_tool,GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(window_tool,"leave_notify_event",G_CALLBACK(cb_hide),NULL);

    remote_viewer(args);

    gtk_main();
    
    return 0;
}
