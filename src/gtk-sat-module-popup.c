/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
    Gpredict: Real-time satellite tracking and orbit prediction program

    Copyright (C)  2001-2008  Alexandru Csete, OZ9AEC.

    Authors: Alexandru Csete <oz9aec@gmail.com>

    Comments, questions and bugreports should be submitted via
    http://sourceforge.net/projects/gpredict/
    More details can be found at the project home page:

            http://gpredict.oz9aec.net/
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, visit http://www.fsf.org/
*/
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include "sgpsdp/sgp4sdp4.h"
#include "sat-log.h"
#include "gpredict-utils.h"
#include "mod-mgr.h"
#include "compat.h"
#include "gtk-sky-glance.h"
#include "sat-cfg.h"
#ifdef HAVE_CONFIG_H
#  include <build-config.h>
#endif
#include "gtk-sat-module.h"
#include "gtk-sat-module-tmg.h"
#include "gtk-sat-module-popup.h"
#include "gtk-rig-ctrl.h"
#include "gtk-rot-ctrl.h"
#include "config-keys.h"


extern GtkWidget *app; /* in main.c */


static void config_cb        (GtkWidget *menuitem, gpointer data);
static void clone_cb         (GtkWidget *menuitem, gpointer data);
static void docking_state_cb (GtkWidget *menuitem, gpointer data);
static void screen_state_cb  (GtkWidget *menuitem, gpointer data);
static void sky_at_glance_cb (GtkWidget *menuitem, gpointer data);
static void tmgr_cb          (GtkWidget *menuitem, gpointer data);
static void rigctrl_cb       (GtkWidget *menuitem, gpointer data);
static void rotctrl_cb       (GtkWidget *menuitem, gpointer data);
static void delete_cb        (GtkWidget *menuitem, gpointer data);
static void close_cb         (GtkWidget *menuitem, gpointer data);
static void name_changed     (GtkWidget *widget, gpointer data);
static void destroy_rotctrl  (GtkWidget *window, gpointer data);
static void destroy_rigctrl  (GtkWidget *window, gpointer data);
static gint window_delete    (GtkWidget *widget, GdkEvent *event, gpointer data);



/** \brief Create and run GtkSatModule popup menu.
 *  \param module The module that should have the popup menu attached to it.
 *
 * This function ctreates and executes a popup menu that is related to a
 * GtkSatModule widget. The module must be a valid GtkSatModule, since it makes
 * no sense whatsoever to have this kind of popup menu without a GtkSatModule
 * parent.
 *
 */
void
gtk_sat_module_popup (GtkSatModule *module)
{
	GtkWidget        *menu;
	GtkWidget        *menuitem;
	GtkWidget        *image;
	gchar            *buff;



	if ((module == NULL) || !IS_GTK_SAT_MODULE (module)) {
		sat_log_log (SAT_LOG_LEVEL_BUG,
					 _("%s:%d: %s called with NULL parameter!"),
					 __FILE__, __LINE__, __FUNCTION__);

		return;
	}

	menu = gtk_menu_new ();

	if (module->state == GTK_SAT_MOD_STATE_DOCKED) {

		menuitem = gtk_image_menu_item_new_with_label (_("Detach module"));
		buff = icon_file_name ("gpredict-notebook.png");
		image = gtk_image_new_from_file (buff);
		g_free (buff);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect (menuitem, "activate",
						  G_CALLBACK (docking_state_cb), module);
	}
	else {

		menuitem = gtk_image_menu_item_new_with_label (_("Attach module"));
		buff = icon_file_name ("gpredict-notebook.png");
		image = gtk_image_new_from_file (buff);
		g_free (buff);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect (menuitem, "activate",
						  G_CALLBACK (docking_state_cb), module);

	}

	if (module->state == GTK_SAT_MOD_STATE_FULLSCREEN) {

		menuitem = gtk_image_menu_item_new_with_label (_("Leave fullscreen"));
		image = gtk_image_new_from_stock (GTK_STOCK_LEAVE_FULLSCREEN,
										  GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect (menuitem, "activate",
						  G_CALLBACK (screen_state_cb), module);
	}
	else {
		menuitem = gtk_image_menu_item_new_with_label (_("Fullscreen"));
		image = gtk_image_new_from_stock (GTK_STOCK_FULLSCREEN,
										  GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
		g_signal_connect (menuitem, "activate",
						  G_CALLBACK (screen_state_cb), module);
	}

	/* separator */
	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* sky at a glance */
	menuitem = gtk_image_menu_item_new_with_label (_("Sky at a glance"));
	buff = icon_file_name ("gpredict-planner-small.png");
	image = gtk_image_new_from_file (buff);
	g_free (buff);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (sky_at_glance_cb), module);

	/* time manager */
	menuitem = gtk_image_menu_item_new_with_label (_("Time Controller"));
	buff = icon_file_name ("gpredict-clock-small.png");
	image = gtk_image_new_from_file (buff);
	g_free (buff);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect (menuitem, "activate", G_CALLBACK (tmgr_cb), module);
    
    /* separator */
    menuitem = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

    /* Radio Control */
    menuitem = gtk_image_menu_item_new_with_label (_("Radio Control"));
    buff = icon_file_name ("gpredict-oscilloscope-small.png");
    image = gtk_image_new_from_file (buff);
    g_free (buff);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
    g_signal_connect (menuitem, "activate", G_CALLBACK (rigctrl_cb), module);
    
    /* Antenna Control */
    menuitem = gtk_image_menu_item_new_with_label (_("Antenna Control"));
    buff = icon_file_name ("gpredict-antenna-small.png");
    image = gtk_image_new_from_file (buff);
    g_free (buff);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
    g_signal_connect (menuitem, "activate", G_CALLBACK (rotctrl_cb), module);
     
	/* separator */
	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* configure */
	menuitem = gtk_image_menu_item_new_with_label (_("Configure"));
	image = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES,
									  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (config_cb), module);

	/* clone */
	menuitem = gtk_image_menu_item_new_with_label (_("Clone..."));
	image = gtk_image_new_from_stock (GTK_STOCK_COPY,
									  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (clone_cb), module);

	/* separator */
	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* delete module */
	menuitem = gtk_image_menu_item_new_with_label (_("Delete"));
	image = gtk_image_new_from_stock (GTK_STOCK_DELETE,
									  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (delete_cb), module);

	/* close */
	menuitem = gtk_image_menu_item_new_with_label (_("Close"));
	image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
									  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (close_cb), module);

	gtk_widget_show_all (menu);

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
					0, gdk_event_get_time (NULL));
}



/** \brief Configure module.
 *
 * This function is called when the user selects the configure
 * menu item in the GtkSatModule popup menu. It is a simple
 * wrapper for gtk_sat_module_config_cb
 *
 */
static void
config_cb        (GtkWidget *menuitem, gpointer data)
{
    GtkSatModule *module = GTK_SAT_MODULE (data);
    
    if (module->rigctrlwin || module->rotctrlwin) {
        GtkWidget *dialog;
        /* FIXME: should offer option to close controllers */
        dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         _("A module can not be configured while the "\
                                           "radio or rotator controller is active.\n\n"\
                                           "Please close the radio and rotator controllers "\
                                           "and try again."));
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
    }
    else {
	   gtk_sat_module_config_cb (menuitem, data);
    }
}



/** \brief Clone module.
 *
 * This function is called when the user selects the clone
 * menu item in the GtkSatModule popup menu. the function creates
 * a dialog in which the user is asked for a new module name.
 * When a valid module name is available and the user clicks on OK,
 * an exact copy of the currwent module is created.
 * By default, the nes module will be opened but the user has the
 * possibility to override this in the dialog window.
 *
 */
static void
clone_cb         (GtkWidget *menuitem, gpointer data)
{
	GtkWidget    *dialog;
	GtkWidget    *entry;
	GtkWidget    *label;
	GtkWidget    *toggle;
	GtkTooltips  *tooltips;
	guint         response;
	GtkSatModule *module = GTK_SAT_MODULE (data);
	GtkSatModule *newmod;
	gchar        *source,*target;
	gchar        *icon;      /* icon file name */
	gchar        *title;     /* window title */


	dialog = gtk_dialog_new_with_buttons (_("Clone Module"),
										  GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (module))),
										  GTK_DIALOG_MODAL |
										  GTK_DIALOG_DESTROY_WITH_PARENT,
										  GTK_STOCK_CANCEL,
										  GTK_RESPONSE_CANCEL,
										  GTK_STOCK_OK,
										  GTK_RESPONSE_OK,
										  NULL);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

	/* label */
	label = gtk_label_new (_("Name of new module:"));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, FALSE, FALSE, 0);

	/* name entry */
	entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (entry), 25);
	gtk_entry_set_text (GTK_ENTRY (entry), module->name);
	tooltips = gtk_tooltips_new ();
	gtk_tooltips_set_tip (tooltips, entry,
						  _("Enter a short name for this module.\n"\
							"Allowed characters: 0..9, a..z, A..Z, - and _"),
						  _("The name will be used to identify the module "\
							"and it is also used a file name for saving the data."\
							"Max length is 25 characters."));

	/* attach changed signal so that we can enable OK button when
	   a proper name has been entered
	   oh, btw. disable OK button to begin with....
	*/
	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
									   GTK_RESPONSE_OK,
									   FALSE);
	g_signal_connect (entry, "changed", G_CALLBACK (name_changed), dialog);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
						entry, FALSE, FALSE, 0);


	/* check button */
	toggle = gtk_check_button_new_with_label (_("Open module when created"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), TRUE);
	tooltips = gtk_tooltips_new ();
	gtk_tooltips_set_tip (tooltips, toggle,
						  _("If checked, the new module will be opened "\
							"after it has been created"),
						  NULL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
						toggle, FALSE, FALSE, 20);
	

	gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);

	/* run dialog */
	response = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (response) {

	case GTK_RESPONSE_OK:
		sat_log_log (SAT_LOG_LEVEL_MSG,
					 _("%s:%d: Cloning %s => %s"),
					 __FILE__, __LINE__, module->name,
					 gtk_entry_get_text (GTK_ENTRY (entry)));

		/* build full file names */
		source = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S,
							  ".gpredict2", G_DIR_SEPARATOR_S,
							  "modules", G_DIR_SEPARATOR_S,
							  module->name, ".mod", NULL);
		target = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S,
							  ".gpredict2", G_DIR_SEPARATOR_S,
							  "modules", G_DIR_SEPARATOR_S,
							  gtk_entry_get_text (GTK_ENTRY (entry)),
							  ".mod", NULL);

		/* copy file */
		if (gpredict_file_copy (source, target)) {
			sat_log_log (SAT_LOG_LEVEL_ERROR,
						 _("%s:%d: Failed to clone %s."),
						 __FILE__, __LINE__, module->name);
		}
		else {
			sat_log_log (SAT_LOG_LEVEL_MSG,
						 _("%s:%d: Successfully cloned %s."),
						 __FILE__, __LINE__, module->name);

			/* open module if requested */
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle))) {
				
				newmod = GTK_SAT_MODULE (gtk_sat_module_new (target));
				newmod->state = module->state;

				if (newmod->state == GTK_SAT_MOD_STATE_DOCKED) {

					/* add to module manager */
					mod_mgr_add_module (GTK_WIDGET (newmod), TRUE);

					/* try to reproduce divisions betwenn views */
					switch (module->layout) {

					case GTK_SAT_MOD_LAYOUT_2:
						gtk_paned_set_position (GTK_PANED (newmod->vpaned),
												gtk_paned_get_position (GTK_PANED (module->vpaned)));
						break;

					case GTK_SAT_MOD_LAYOUT_3:
					case GTK_SAT_MOD_LAYOUT_4:
						gtk_paned_set_position (GTK_PANED (newmod->vpaned),
												gtk_paned_get_position (GTK_PANED (module->vpaned)));
						gtk_paned_set_position (GTK_PANED (newmod->hpaned),
												gtk_paned_get_position (GTK_PANED (module->hpaned)));
						break;

					default:
						break;

					}

				}
				else {
					/* add to module manager */
					mod_mgr_add_module (GTK_WIDGET (newmod), FALSE);

					/* create window */
					newmod->win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
					gtk_window_set_title (GTK_WINDOW (newmod->win),
										  newmod->name);
					title = g_strconcat ("GPREDICT: ",
										 newmod->name,
										 " (", newmod->qth->name, ")",
										 NULL);
					gtk_window_set_title (GTK_WINDOW (newmod->win), title);
					g_free (title);

					/* use size of source module */
					gtk_window_set_default_size (GTK_WINDOW (newmod->win),
												 GTK_WIDGET (module)->allocation.width,
												 GTK_WIDGET (module)->allocation.height);

					g_signal_connect (G_OBJECT (newmod->win), "configure_event",
									  G_CALLBACK (module_window_config_cb), newmod);

					/* add module to window */
					gtk_container_add (GTK_CONTAINER (newmod->win),
									   GTK_WIDGET (newmod));

					/* window icon */
					icon = icon_file_name ("gpredict-icon.png");
					if (g_file_test (icon, G_FILE_TEST_EXISTS)) {
						gtk_window_set_icon_from_file (GTK_WINDOW (newmod->win), icon, NULL);
					}
					g_free (icon);

					/* show window */
					gtk_widget_show_all (newmod->win);

				}
			}
		}

		/* clean up */
		g_free (source);
		g_free (target);

		break;

	case GTK_RESPONSE_CANCEL:
		sat_log_log (SAT_LOG_LEVEL_MSG,
					 _("%s:%d: Cloning cancelled by user."),
					 __FILE__, __LINE__);
		break;

	default:
		sat_log_log (SAT_LOG_LEVEL_MSG,
					 _("%s:%d: Cloning interrupted."),
					 __FILE__, __LINE__);
		break;
	}

	gtk_widget_destroy (dialog);

}


/** \brief Toggle dockig state.
 *
 * This function is called when the user selects the (Un)Dock menu
 * item in the GtkSatModule popup menu. If the current module state
 * is DOCKED, the module will be undocked and moved into it's own,
 * GtkWindow. If the current module state is WINDOW or FULLSCREEN,
 * the module will be docked.
 *
 * The text of the menu item will be changed corresponding to the
 * action that has been performed.
 */
static void
docking_state_cb (GtkWidget *menuitem, gpointer data)
{
	GtkWidget *module = GTK_WIDGET (data);
	gint       w,h;
	gchar     *icon;      /* icon file name */
	gchar     *title;     /* window title */



	switch (GTK_SAT_MODULE (module)->state) {

	case GTK_SAT_MOD_STATE_DOCKED:

        /* get stored size; use size from main window if size not explicitly stoed */
        if (g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata,
								MOD_CFG_GLOBAL_SECTION,
								MOD_CFG_WIN_WIDTH,
								NULL)) {
            w = g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata,
										MOD_CFG_GLOBAL_SECTION,
										MOD_CFG_WIN_WIDTH,
										NULL);
        }
        else {
            w = module->allocation.width;
        }
        if (g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata,
								MOD_CFG_GLOBAL_SECTION,
								MOD_CFG_WIN_HEIGHT,
								NULL)) {
            h = g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata,
										MOD_CFG_GLOBAL_SECTION,
										MOD_CFG_WIN_HEIGHT,
										NULL);
        }
        else {
            h = module->allocation.height;
        }
        
		/* increase reference count of module */
		g_object_ref (module);

		/* we don't need the positions */
		GTK_SAT_MODULE (module)->vpanedpos = -1;
		GTK_SAT_MODULE (module)->hpanedpos = -1;

		/* undock from mod-mgr */
		mod_mgr_undock_module (module);

		/* create window */
		GTK_SAT_MODULE (module)->win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		title = g_strconcat ("GPREDICT: ",
							 GTK_SAT_MODULE (module)->name,
							 " (", GTK_SAT_MODULE (module)->qth->name, ")",
							 NULL);
		gtk_window_set_title (GTK_WINDOW (GTK_SAT_MODULE (module)->win), title);
		g_free (title);
		gtk_window_set_default_size (GTK_WINDOW (GTK_SAT_MODULE (module)->win), w, h);
        g_signal_connect (G_OBJECT (GTK_SAT_MODULE (module)->win), "configure_event",
                          G_CALLBACK (module_window_config_cb), module);

		/* window icon */
		icon = icon_file_name ("gpredict-icon.png");
		if (g_file_test (icon, G_FILE_TEST_EXISTS)) {
			gtk_window_set_icon_from_file (GTK_WINDOW (GTK_SAT_MODULE (module)->win), icon, NULL);
		}
		g_free (icon);

        /* move window to stored position if requested by configuration */
        if (sat_cfg_get_bool (SAT_CFG_BOOL_MOD_WIN_POS) &&
            g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata,
								MOD_CFG_GLOBAL_SECTION,
								MOD_CFG_WIN_POS_X,
								NULL) &&
            g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata,
								MOD_CFG_GLOBAL_SECTION,
								MOD_CFG_WIN_POS_Y,
								NULL)) {
                
            gtk_window_move (GTK_WINDOW (GTK_SAT_MODULE (module)->win),
                             g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata,
													 MOD_CFG_GLOBAL_SECTION,
													 MOD_CFG_WIN_POS_X, NULL),
                             g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata,
													 MOD_CFG_GLOBAL_SECTION,
													 MOD_CFG_WIN_POS_Y,
													 NULL));

        }

		/* add module to window */
		gtk_container_add (GTK_CONTAINER (GTK_SAT_MODULE (module)->win), module);

		/* change internal state */
		GTK_SAT_MODULE (module)->state = GTK_SAT_MOD_STATE_WINDOW;

        /* store new state in configuration */
        g_key_file_set_integer (GTK_SAT_MODULE (module)->cfgdata,
								MOD_CFG_GLOBAL_SECTION,
								MOD_CFG_STATE,
								GTK_SAT_MOD_STATE_WINDOW);

		/* decrease reference count of module */
		g_object_unref (module);

		/* show window */
		gtk_widget_show_all (GTK_SAT_MODULE (module)->win);
        
		/* reparent time manager window if visible */
		if (GTK_SAT_MODULE (module)->tmgActive) {
			gtk_window_set_transient_for (GTK_WINDOW (GTK_SAT_MODULE (module)->tmgWin),
										  GTK_WINDOW (GTK_SAT_MODULE (module)->win));
		}

		break;

	case GTK_SAT_MOD_STATE_WINDOW:
	case GTK_SAT_MOD_STATE_FULLSCREEN:

		/* increase referene count */
		g_object_ref (module);

		/* store paned positions */
		if (GTK_SAT_MODULE (module)->layout == GTK_SAT_MOD_LAYOUT_2) {
			GTK_SAT_MODULE (module)->vpanedpos = gtk_paned_get_position (GTK_PANED (GTK_SAT_MODULE (module)->vpaned));
		}
		else if ((GTK_SAT_MODULE (module)->layout == GTK_SAT_MOD_LAYOUT_3) ||
				 (GTK_SAT_MODULE (module)->layout == GTK_SAT_MOD_LAYOUT_4)) {

			GTK_SAT_MODULE (module)->vpanedpos = gtk_paned_get_position (GTK_PANED (GTK_SAT_MODULE (module)->vpaned));
			GTK_SAT_MODULE (module)->hpanedpos = gtk_paned_get_position (GTK_PANED (GTK_SAT_MODULE (module)->hpaned));
		}
        
		/* reparent time manager window if visible */
		if (GTK_SAT_MODULE (module)->tmgActive) {
			gtk_window_set_transient_for (GTK_WINDOW (GTK_SAT_MODULE (module)->tmgWin),
										  GTK_WINDOW (app));
		}

		/* remove module from window, destroy window */
		gtk_container_remove (GTK_CONTAINER (GTK_SAT_MODULE (module)->win), module);
		gtk_widget_destroy (GTK_SAT_MODULE (module)->win);
		GTK_SAT_MODULE (module)->win = NULL;

		/* dock into mod-mgr */
		mod_mgr_dock_module (module);

		/* change internal state */
		GTK_SAT_MODULE (module)->state = GTK_SAT_MOD_STATE_DOCKED;
        
        /* store new state in configuration */
        g_key_file_set_integer (GTK_SAT_MODULE (module)->cfgdata,
								MOD_CFG_GLOBAL_SECTION,
								MOD_CFG_STATE,
								GTK_SAT_MOD_STATE_DOCKED);

		/* decrease reference count of module */
		g_object_unref (module);


		break;

	default:

		sat_log_log (SAT_LOG_LEVEL_BUG,
					 _("%s:%d: Unknown module state: %d"),
					 __FILE__, __LINE__, GTK_SAT_MODULE (module)->state);
		break;

	}


}



/** \brief Toggle screen state.
 *
 * This function is intended to toggle between FULLSCREEN
 * and WINDOW state.
 */
static void
screen_state_cb  (GtkWidget *menuitem, gpointer data)
{
	GtkWidget *module = GTK_WIDGET (data);
    gint       w,h;
	gchar     *icon;      /* icon file name */
	gchar     *title;     /* window title */


	switch (GTK_SAT_MODULE (module)->state) {

	case GTK_SAT_MOD_STATE_DOCKED:

		/* increase reference count of module */
		g_object_ref (module);

		/* we don't need the positions */
		GTK_SAT_MODULE (module)->vpanedpos = -1;
		GTK_SAT_MODULE (module)->hpanedpos = -1;

		/* undock from mod-mgr */
		mod_mgr_undock_module (module);

		/* create window */
		GTK_SAT_MODULE (module)->win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		title = g_strconcat ("GPREDICT: ",
							 GTK_SAT_MODULE (module)->name,
							 " (", GTK_SAT_MODULE (module)->qth->name, ")",
							 NULL);
		gtk_window_set_title (GTK_WINDOW (GTK_SAT_MODULE (module)->win), title);
		g_free (title);

		/* window icon */
		icon = icon_file_name ("gpredict-icon.png");
		if (g_file_test (icon, G_FILE_TEST_EXISTS)) {
			gtk_window_set_icon_from_file (GTK_WINDOW (GTK_SAT_MODULE (module)->win), icon, NULL);
		}
		g_free (icon);

		/* add module to window */
		gtk_container_add (GTK_CONTAINER (GTK_SAT_MODULE (module)->win), module);

		/* change internal state */
		GTK_SAT_MODULE (module)->state = GTK_SAT_MOD_STATE_FULLSCREEN;

		/* decrease reference count of module */
		g_object_unref (module);

		gtk_window_fullscreen (GTK_WINDOW (GTK_SAT_MODULE (module)->win));

		/* show window */
		gtk_widget_show_all (GTK_SAT_MODULE (module)->win);
        
		/* reparent time manager window if visible */
		if (GTK_SAT_MODULE (module)->tmgActive) {
			gtk_window_set_transient_for (GTK_WINDOW (GTK_SAT_MODULE (module)->tmgWin),
										  GTK_WINDOW (GTK_SAT_MODULE (module)->win));
		}

		break;


	case GTK_SAT_MOD_STATE_WINDOW:

		/* we don't need the positions */
		GTK_SAT_MODULE (module)->vpanedpos = -1;
		GTK_SAT_MODULE (module)->hpanedpos = -1;

		/* change internal state */
		GTK_SAT_MODULE (module)->state = GTK_SAT_MOD_STATE_FULLSCREEN;
		gtk_window_fullscreen (GTK_WINDOW (GTK_SAT_MODULE (module)->win));
		gtk_window_set_default_size (GTK_WINDOW (GTK_SAT_MODULE (module)->win), 800, 600);

		break;


	case GTK_SAT_MOD_STATE_FULLSCREEN:
        
		/* store paned positions */
		if (GTK_SAT_MODULE (module)->layout == GTK_SAT_MOD_LAYOUT_2) {
			GTK_SAT_MODULE (module)->vpanedpos = gtk_paned_get_position (GTK_PANED (GTK_SAT_MODULE (module)->vpaned));
		}
		else if ((GTK_SAT_MODULE (module)->layout == GTK_SAT_MOD_LAYOUT_3) ||
				 (GTK_SAT_MODULE (module)->layout == GTK_SAT_MOD_LAYOUT_4)) {

			GTK_SAT_MODULE (module)->vpanedpos = gtk_paned_get_position (GTK_PANED (GTK_SAT_MODULE (module)->vpaned));
			GTK_SAT_MODULE (module)->hpanedpos = gtk_paned_get_position (GTK_PANED (GTK_SAT_MODULE (module)->hpaned));
		}

		/* change internal state */
		GTK_SAT_MODULE (module)->state = GTK_SAT_MOD_STATE_WINDOW;
        gtk_window_unfullscreen (GTK_WINDOW (GTK_SAT_MODULE (module)->win));

        /* get stored size; use some standard size if not explicitly specified */
        if (g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_WIDTH, NULL)) {
            w = g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_WIDTH, NULL);
        }
        else {
            w = 800;
        }
        if (g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_HEIGHT, NULL)) {
            h = g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_HEIGHT, NULL);
        }
        else {
            h = 600;
        }
        gtk_window_set_default_size (GTK_WINDOW (GTK_SAT_MODULE (module)->win), w, h);

        /* move window to stored position if requested by configuration */
        if (sat_cfg_get_bool (SAT_CFG_BOOL_MOD_WIN_POS) &&
            g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_POS_X, NULL) &&
            g_key_file_has_key (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_POS_Y, NULL)) {
                
            gtk_window_move (GTK_WINDOW (GTK_SAT_MODULE (module)->win),
                             g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_POS_X, NULL),
                                     g_key_file_get_integer (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_POS_Y, NULL));

            }

        /* store new state in configuration */
        g_key_file_set_integer (GTK_SAT_MODULE (module)->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_STATE, GTK_SAT_MOD_STATE_WINDOW);

		break;

	default:

		sat_log_log (SAT_LOG_LEVEL_BUG,
					 _("%s:%d: Unknown module state: %d"),
					 __FILE__, __LINE__, GTK_SAT_MODULE (module)->state);
		break;

	}

}


/** \brief Invoke Sky-at-glance.
 *
 * This function is a shortcut to the sky at glance function
 * in that it will make the predictions with the satellites
 * tracked in the current module.
 */
static void
sky_at_glance_cb (GtkWidget *menuitem, gpointer data)
{
	GtkSatModule *module = GTK_SAT_MODULE (data);
	GtkWidget    *skg;
	GtkWidget    *window;
	gchar        *buff;


	/* create window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	buff = g_strdup_printf (_("The sky at a glance (%s)"), module->name);
	gtk_window_set_title (GTK_WINDOW (window), buff);
	g_free (buff);
	g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (window_delete), NULL);    

	/* window icon */
	buff = icon_file_name ("gpredict-planner.png");
	gtk_window_set_icon_from_file (GTK_WINDOW (window), buff, NULL);
	g_free (buff);

	/* if module is busy wait until done then go on */
	while (module->busy)
		g_usleep (1000);

	/* create sky at a glance widget */
	module->busy = TRUE;
    
    if (sat_cfg_get_bool (SAT_CFG_BOOL_PRED_USE_REAL_T0)) {
        skg = gtk_sky_glance_new (module->satellites, module->qth, 0.0);
    }
    else {
        skg = gtk_sky_glance_new (module->satellites, module->qth, module->tmgCdnum);
    }
    
	module->busy = FALSE;

	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_container_add (GTK_CONTAINER (window), skg);

	gtk_widget_show_all (window);

}

/** \brief Open time manager. */
static void
tmgr_cb          (GtkWidget *menuitem, gpointer data)
{
	GtkSatModule *module = GTK_SAT_MODULE (data);

	tmg_create (module);
}

/** \brief Open Radio control window. 
 * \param menuitem The menuitem that was selected.
 * \param data Pointer the GtkSatModule.
 */
static void
rigctrl_cb          (GtkWidget *menuitem, gpointer data)
{
    GtkSatModule *module = GTK_SAT_MODULE (data);
    gchar *buff;
    
    if (module->rigctrlwin != NULL) {
        /* there is already a roto controller for this module */
        gtk_window_present (GTK_WINDOW (module->rigctrlwin));
        return;
    }

    module->rigctrl = gtk_rig_ctrl_new (module);
    
    if (module->rigctrl == NULL) {
        /* gtk_rot_ctrl_new returned NULL becasue no radios are configured */
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW (app),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                         _("You have no radio configuration!\n"\
                                           "Please configure a radio first.")
                                        );
        g_signal_connect_swapped (dialog, "response", 
                                  G_CALLBACK (gtk_widget_destroy), dialog);
        gtk_window_set_title (GTK_WINDOW (dialog), _("ERROR"));
        gtk_widget_show_all (dialog);
        
        return;
    }
    
    /* create a window */
    module->rigctrlwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    buff = g_strdup_printf (_("Gpredict Radio Control: %s"), module->name);
    gtk_window_set_title (GTK_WINDOW (module->rigctrlwin), buff);
    g_free (buff);
    g_signal_connect (G_OBJECT (module->rigctrlwin), "delete_event",
                      G_CALLBACK (window_delete), NULL);
    g_signal_connect (G_OBJECT (module->rigctrlwin), "destroy",
                      G_CALLBACK (destroy_rigctrl), module);

    /* window icon */
    buff = icon_file_name ("gpredict-oscilloscope.png");
    gtk_window_set_icon_from_file (GTK_WINDOW (module->rigctrlwin), buff, NULL);
    g_free (buff);
    
    gtk_container_add (GTK_CONTAINER (module->rigctrlwin), module->rigctrl);
    
    gtk_widget_show_all (module->rigctrlwin);

}


/** \brief Destroy radio control window.
 * \param window Pointer to the radio control window.
 * \param data Pointer to the GtkSatModule to which this controller is attached.
 * 
 * This function is called automatically when the window is destroyed.
 */
static void
destroy_rigctrl  (GtkWidget *window, gpointer data)
{
    GtkSatModule *module = GTK_SAT_MODULE (data);
    
    module->rigctrlwin = NULL;
    module->rigctrl = NULL;
}


/** \brief Open antenna rotator control window. 
 * \param menuitem The menuitem that was selected.
 * \param data Pointer the GtkSatModule.
 */
static void
rotctrl_cb          (GtkWidget *menuitem, gpointer data)
{
    GtkSatModule *module = GTK_SAT_MODULE (data);
    gchar *buff;
    
    if (module->rotctrlwin != NULL) {
        /* there is already a roto controller for this module */
        gtk_window_present (GTK_WINDOW (module->rotctrlwin));
        return;
    }

    module->rotctrl = gtk_rot_ctrl_new (module);
    
    if (module->rotctrl == NULL) {
        /* gtk_rot_ctrl_new returned NULL becasue no rotators are configured */
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new (GTK_WINDOW (app),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                         _("You have no rotator configuration!\n"\
                                           "Please configure an antenna rotator first.")
                                        );
        g_signal_connect_swapped (dialog, "response", 
                                  G_CALLBACK (gtk_widget_destroy), dialog);
        gtk_window_set_title (GTK_WINDOW (dialog), _("ERROR"));
        gtk_widget_show_all (dialog);
        
        return;
    }
    
    /* create a window */
    module->rotctrlwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    buff = g_strdup_printf (_("Gpredict Rotator Control: %s"), module->name);
    gtk_window_set_title (GTK_WINDOW (module->rotctrlwin), buff);
    g_free (buff);
    g_signal_connect (G_OBJECT (module->rotctrlwin), "delete_event",
                      G_CALLBACK (window_delete), module);
    g_signal_connect (G_OBJECT (module->rotctrlwin), "destroy",
                      G_CALLBACK (destroy_rotctrl), module);

    /* window icon */
    buff = icon_file_name ("gpredict-antenna.png");
    gtk_window_set_icon_from_file (GTK_WINDOW (module->rotctrlwin), buff, NULL);
    g_free (buff);
    
    gtk_container_add (GTK_CONTAINER (module->rotctrlwin), module->rotctrl);
    
    gtk_widget_show_all (module->rotctrlwin);
}


/** \brief Destroy rotator control window.
 * \param window Pointer to the rotator control window.
 * \param data Pointer to the GtkSatModule to which this controller is attached.
 * 
 * This function is called automatically when the window is destroyed.
 */
static void
destroy_rotctrl  (GtkWidget *window, gpointer data)
{
    GtkSatModule *module = GTK_SAT_MODULE (data);
    
    module->rotctrlwin = NULL;
    module->rotctrl    = NULL;
}

/* ensure that deleted top-level windows are destroyed */
static gint
window_delete      (GtkWidget *widget,
                    GdkEvent  *event,
                    gpointer   data)
{
    return FALSE;
}



/** \brief Close module.
 *
 * This function is called when the user selects the close menu
 * item in the GtkSatModule popup menu. It is simply a wrapper
 * for gtk_sat_module_close_cb, which will close the current module.
 */
static void
close_cb         (GtkWidget *menuitem, gpointer data)
{
	gtk_sat_module_close_cb (menuitem, data);
}

/** \brief Close and permanently delete module.
 *
 * This function is called when the user selects the delete menu
 * item in the GtkSatModule popup menu. First it will close the module
 * with gtk_sat_module_close_cb, which will close the current module,
 * whereafter the module file will be deleted from the disk.
 */
static void
delete_cb         (GtkWidget *menuitem, gpointer data)
{
	gchar *file;
	GtkWidget *dialog;
	//GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (data));


	file = g_strconcat (g_get_home_dir (), G_DIR_SEPARATOR_S,
						".gpredict2", G_DIR_SEPARATOR_S,
						"modules", G_DIR_SEPARATOR_S,
						GTK_SAT_MODULE (data)->name, ".mod", NULL);

	gtk_sat_module_close_cb (menuitem, data);


	/* ask user to confirm removal */
	dialog = gtk_message_dialog_new_with_markup 
		(NULL, //GTK_WINDOW (parent),
		 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		 GTK_MESSAGE_QUESTION,
		 GTK_BUTTONS_YES_NO,
		 _("This operation will permanently delete\n<b>%s</b>\n"\
		   "from the disk.\nDo you you want to proceed?"),
		 file);

	switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

	case GTK_RESPONSE_YES:

		if (g_remove (file)) {
			sat_log_log (SAT_LOG_LEVEL_ERROR,
						 _("%s:%d: Failed to delete %s."),
						 __FILE__, __LINE__, file);
		}
		else {
			sat_log_log (SAT_LOG_LEVEL_ERROR,
						 _("%s:%d: %s deleted permanently."),
						 __FILE__, __LINE__, file);
		}
		break;

	default:
		break;
	}

	gtk_widget_destroy (dialog);

	g_free (file);
}


/** \brief Manage name changes.
 *
 * This function is called when the contents of the name entry changes.
 * The primary purpose of this function is to check whether the char length
 * of the name is greater than zero, if yes enable the OK button of the dialog.
 */
static void
name_changed          (GtkWidget *widget, gpointer data)
{
	const gchar *text;
	gchar       *entry, *end, *j;
	gint         len, pos;
	GtkWidget   *dialog = GTK_WIDGET (data);


	/* step 1: ensure that only valid characters are entered
	   (stolen from xlog, tnx pg4i)
	*/
	entry = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1);
	if ((len = g_utf8_strlen (entry, -1)) > 0)
		{
			end = entry + g_utf8_strlen (entry, -1);
			for (j = entry; j < end; ++j)
				{
					switch (*j)
						{
						case '0' ... '9':
						case 'a' ... 'z':
						case 'A' ... 'Z':
						case '-':
						case '_':
							break;
						default:
							gdk_beep ();
							pos = gtk_editable_get_position (GTK_EDITABLE (widget));
							gtk_editable_delete_text (GTK_EDITABLE (widget),
													  pos, pos+1);
							break;
						}
				}
		}


	/* step 2: if name seems all right, enable OK button */
	text = gtk_entry_get_text (GTK_ENTRY (widget));

	if (g_utf8_strlen (text, -1) > 0) {
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
										   GTK_RESPONSE_OK,
										   TRUE);
	}
	else {
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog),
										   GTK_RESPONSE_OK,
										   FALSE);
	}
}





/** \brief Snoop window position and size when main window receives configure event.
 *  \param widget Pointer to the module window.
 *  \param event  Pointer to the event structure.
 *  \param data   Pointer to user data, in this case the module.
 *
 * This function is used to trap configure events in order to store the current
 * position and size of the module window.
 *
 * \note unfortunately GdkEventConfigure ignores the window gravity, while
 *       the only way we have of setting the position doesn't. We have to
 *       call get_position because it does pay attention to the gravity.
 *
 * \note The logic in the code has been borrowed from gaim/pidgin http://pidgin.im/
 *
 */
gboolean
module_window_config_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    gint x, y;
    GtkSatModule *module = GTK_SAT_MODULE (data);


    /* data is only useful when window is visible */
    if (GTK_WIDGET_VISIBLE (widget))
        gtk_window_get_position (GTK_WINDOW (widget), &x, &y);
    else
        return FALSE; /* carry on normally */
 
#ifdef G_OS_WIN32
    /* Workaround for GTK+ bug # 169811 - "configure_event" is fired
    when the window is being maximized */
    if (gdk_window_get_state (widget->window) & GDK_WINDOW_STATE_MAXIMIZED) {
        return FALSE;
    }
#endif

    /* don't save off-screen positioning */
    if (x + event->width < 0 || y + event->height < 0 ||
        x > gdk_screen_width() || y > gdk_screen_height()) {
        return FALSE; /* carry on normally */
    }

    /* store the position and size */
    g_key_file_set_integer (module->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_POS_X, x);
    g_key_file_set_integer (module->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_POS_Y, y);
    g_key_file_set_integer (module->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_WIDTH, event->width);
    g_key_file_set_integer (module->cfgdata, MOD_CFG_GLOBAL_SECTION, MOD_CFG_WIN_HEIGHT, event->height);

    /* continue to handle event normally */
    return FALSE;
}
