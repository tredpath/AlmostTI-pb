/*
 * QNXMenu.cpp
 *
 *  Created on: 2012-07-12
 *      Author: Travis Redpath
 */

#include <bps/deviceinfo.h>

#include "QNXMenu.h"
#include "../TI85.h"
#include "../../EMULib/QNX/LibQNX.h"

#include <stdlib.h>
#include <strings.h>

extern const char *Title;	/* Program title */

static std::vector<std::string> vsList;
static std::vector<std::string> sortedvecList;


std::vector<std::string> sortAlpha(std::vector<std::string> sortThis)
{
	int swap;
	std::string temp;

	do
	{
		swap = 0;
		for (size_t count = 0; count < sortThis.size() - 1; count++)
		{
			if (sortThis.at(count) > sortThis.at(count + 1))
			{
				temp = sortThis.at(count);
				sortThis.at(count) = sortThis.at(count + 1);
				sortThis.at(count + 1) = temp;
				swap = 1;
			}
		}
	} while (swap != 0);

	return sortThis;
}

int UpdateRomListError()
{
	int retval = 1;
	dialog_instance_t alert_dialog = 0;
	dialog_request_events(0);
	if (dialog_create_alert(&alert_dialog) != BPS_SUCCESS)
		fprintf(stderr, "Failed to create alert dialog.");
	dialog_set_title_text(alert_dialog, "Almost-TI Error Report");
	if (dialog_set_alert_html_message_text(alert_dialog, "<u><b>ERROR:</b> You do not have any ROMS!<br></u><b>Add TI ROMS to:</b> <br>\"<u>shared/misc/TI/ROM</u>\"<br>using either <i>WiFi sharing</i> or <i>Desktop Software.</i>") != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to set alert dialog message text.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	char* cancel_button_context = (char*)"Cancelled";
	char* ok_button_context = (char*)"OK";

	dialog_add_button(alert_dialog, "Close", true, cancel_button_context, true);
	dialog_add_button(alert_dialog, "Refresh", true, ok_button_context, true);

	if (dialog_show(alert_dialog) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to show alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	while(1)
	{
		bps_event_t* event = NULL;
		bps_get_event(&event, -1);

		if (event)
		{
			if (bps_event_get_domain(event) == dialog_get_domain())
			{
				const char* context = dialog_event_get_selected_context(event);
				if (strcmp(context, cancel_button_context) == 0)
					retval = 0;
				break;
			}
		}
	}
	if (alert_dialog)
		dialog_destroy(alert_dialog);
	return retval;
}

int UpdateRomList(int allowRetry)
{
	DIR* dirp;
	struct dirent* direntp;

SCANDIR:
	vsList.clear();
	sortedvecList.clear();
	//vsList.push_back(Title);

	dirp = opendir("/accounts/1000/shared/misc/TI/ROM/");
	if (dirp != NULL)
	{
		for (;;)
		{
			direntp = readdir(dirp);
			if (direntp == NULL)
				break;
			fprintf(stderr, "FILE: '%s' \n", direntp->d_name);
			std::string tmp = direntp->d_name;
			if (strcmp(direntp->d_name, ".") == 0)
				continue;
			if (strcmp(direntp->d_name, "..") == 0)
				continue;
			int i;
			for (i = 0; Config[i].ROMFile; i++)
				if (strcasecmp(tmp.c_str(), Config[i].ROMFile) == 0)
				//if (tmp.compare(Config[i].ROMFile) == 0)
					break;
			if (Config[i].ROMFile)
				vsList.push_back(direntp->d_name);
		}
	}
	else
		fprintf(stderr, "dirp is NULL ...\n");
	fprintf(stderr, "number of files %d\n", vsList.size());
	closedir(dirp);

	if (vsList.size() == 0)
	{
		if (allowRetry && UpdateRomListError())
			goto SCANDIR;
		return 0;
	}

	sortedvecList = sortAlpha(vsList);
	return 1;
}

int msg_box1(const char *title, const char *message)
{
	dialog_instance_t alert_dialog = 0;
	dialog_request_events(0);
	if (dialog_create_alert(&alert_dialog) != BPS_SUCCESS)
		fprintf(stderr, "Failed to create alert dialog.");
	dialog_set_title_text(alert_dialog, title);
	if (dialog_set_alert_html_message_text(alert_dialog, message) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to set alert dialog message text.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	char* cancel_button_context = (char*)"Cancelled";

	if (dialog_add_button(alert_dialog, "OK", true, cancel_button_context, true) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to add button to alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	if (dialog_show(alert_dialog) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to show alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	while(1)
	{
		bps_event_t* event = NULL;
		bps_get_event(&event, -1);

		if (event)
		{
			if (bps_event_get_domain(event) == dialog_get_domain())
				break;
		}
	}
	if (alert_dialog)
		dialog_destroy(alert_dialog);

	return 0;
}

int msg_box2(const char *title, const char *message)
{
	int retval = 0;
	dialog_instance_t alert_dialog = 0;
	dialog_request_events(0);
	if (dialog_create_alert(&alert_dialog) != BPS_SUCCESS)
		fprintf(stderr, "Failed to create alert dialog.");
	dialog_set_title_text(alert_dialog, title);
	if (dialog_set_alert_html_message_text(alert_dialog, message) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to set alert dialog message text.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	char* cancel_button_context = (char*)"Cancelled";
	char* ok_button_context = (char*)"Okay";

	if (dialog_add_button(alert_dialog, "Try Anyway", true, ok_button_context, true) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to add button to alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	if (dialog_add_button(alert_dialog, "Cancel", true, cancel_button_context, true) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to add button to alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	if (dialog_show(alert_dialog) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to show alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	while(1)
	{
		bps_event_t* event = NULL;
		bps_get_event(&event, -1);

		if (event)
		{
			if (bps_event_get_domain(event) == dialog_get_domain())
			{
				const char* context = dialog_event_get_selected_context(event);
				if (stricmp(context, ok_button_context) == 0)
					retval = OK_BUTTON;
				else if (stricmp(context, cancel_button_context) == 0)
					retval = CANCEL_BUTTON;
				break;
			}
		}
	}
	if (alert_dialog)
		dialog_destroy(alert_dialog);

	return retval;
}

int showRomInfo()
{
	dialog_instance_t alert_dialog = 0;
	dialog_request_events(0);
	if (dialog_create_alert(&alert_dialog) != BPS_SUCCESS)
		fprintf(stderr, "Failed to create alert dialog.");
	dialog_set_title_text(alert_dialog, "ROM Info");
	char info[2048];
	char temp[128];
	strcpy(info, "<b><u>ROM File : Required File Size</u></b><br>");
	int M = 0;
	while (Config[M].ROMFile)
	{
		sprintf(temp, "%s: %d bytes<br>", Config[M].ROMFile, Config[M].ROMSize);
		strcat(info, temp);
		M++;
	}
	dialog_set_alert_html_message_text(alert_dialog, info);

	char* cancel_button_context = (char*)"Cancelled";

	dialog_add_button(alert_dialog, "OK", true, cancel_button_context, true);
	dialog_set_size(alert_dialog, DIALOG_SIZE_MEDIUM);

	dialog_show(alert_dialog);

	while(1)
	{
		bps_event_t* event = NULL;
		bps_get_event(&event, -1);

		if (event)
		{
			if (bps_event_get_domain(event) == dialog_get_domain())
				break;
		}
	}
	if (alert_dialog)
		dialog_destroy(alert_dialog);

	return 0;
}

int AutoLoadRom(int &M, bool showInfo)
{
	int status = 0;
	M = -1;

	const char** list = 0;
	size_t count = 0;
	list = (const char**)malloc((sortedvecList.size() + (showInfo ? 2 : 0)) * sizeof(char*));
	dialog_request_events(0);

	for(;;)
	{
		if (count >= sortedvecList.size())
			break;
		fprintf(stderr, "%d \n", count);
		list[count] = sortedvecList[count].c_str();
		count++;
	}
	if (showInfo)
	{
		list[count] = "Information";
		count++;
		list[count] = "ROM File Sizes";
		count++;
	}

	dialog_instance_t dialog = 0;
	int i, rc;
	bps_event_t *event;
	int domain = 0;
	const char* label;
	//char romfilename[256];
	dialog_create_popuplist(&dialog);
	dialog_set_popuplist_items(dialog, list, count);
	if (showInfo)
	{
		int indice[] = { sortedvecList.size() };
		dialog_set_popuplist_separator_indices(dialog, (int*)&indice, 1);
	}
	dialog_set_title_text(dialog, Title);

	char* cancel_button_context = (char*)"Cancelled";
	char* okay_button_context = (char*)"Okay";
	dialog_add_button(dialog, DIALOG_CANCEL_LABEL, true, cancel_button_context, true);
	dialog_add_button(dialog, DIALOG_OK_LABEL, true, okay_button_context, true);
	dialog_set_popuplist_multiselect(dialog, false);
	dialog_show(dialog);

	while(1)
	{
		bps_get_event(&event, -1);
		if (event)
		{
			domain = bps_event_get_domain(event);
			if (domain == dialog_get_domain())
			{
				int *response[1];
				int num;
				label = dialog_event_get_selected_label(event);
				if (strcmp(label, DIALOG_OK_LABEL) == 0)
				{
					dialog_event_get_popuplist_selected_indices(event, (int**)response, &num);
					if (num != 0)
					{
						if (*response[0] < sortedvecList.size())
						{
							printf("%s\n", list[*response[0]]);
							fflush(stdout);
							//strcpy(romfilename, list[*response[0]]);
							strcpy(TIFilename, list[*response[0]]);
						}
						else
						{
							showRomInfo();
							dialog_show(dialog);
							bps_free(response[0]);
							continue;
						}
					}
					bps_free(response[0]);
				}
				else
				{
					printf("User has cancelled ISO dialog.");
					free(list);
					return -1;
				}
				break;
			}
		}
	}

	free(list);

	//return WhichTI(romfilename, M);
	return WhichTI(TIFilename, M);
}

void ToggleEffects()
{
	int status = 0;

	const char** list = 0;
	size_t count = 0;
	list = (const char**)malloc(3 * sizeof(char*));
	dialog_request_events(0);

	list[count++] = "Scanlines";
	list[count++] = "Frame Rate";
	list[count++] = "Soften";

	int numSel = BPSGetEffects();
	numSel = ((numSel & EFF_TVLINES) ? 1 : 0) + ((numSel & EFF_SHOWFPS) ? 1 : 0) + ((numSel & EFF_SOFTEN) ? 1 : 0);
	int* selected = (int*)malloc(numSel * sizeof(int));
	int c = 0;
	if (BPSGetEffects() & EFF_TVLINES)
		selected[c++] = 0;
	if (BPSGetEffects() & EFF_SHOWFPS)
		selected[c++] = 1;
	if (BPSGetEffects() & EFF_SOFTEN)
		selected[c++] = 2;

	dialog_instance_t dialog = 0;
	int i, rc;
	bps_event_t *event;
	int domain = 0;
	const char* label;
	char romfilename[256];
	dialog_create_popuplist(&dialog);
	dialog_set_popuplist_items(dialog, list, count);
	dialog_set_popuplist_selected_indices(dialog, selected, numSel);

	char* cancel_button_context = (char*)"Cancelled";
	char* okay_button_context = (char*)"Okay";
	dialog_add_button(dialog, DIALOG_CANCEL_LABEL, true, cancel_button_context, true);
	dialog_add_button(dialog, DIALOG_OK_LABEL, true, okay_button_context, true);
	dialog_set_popuplist_multiselect(dialog, true);
	dialog_set_title_text(dialog, "Toggle Effects");
	dialog_show(dialog);

	while(1)
	{
		bps_get_event(&event, -1);
		if (event)
		{
			domain = bps_event_get_domain(event);
			if (domain == dialog_get_domain())
			{
				int *response;
				int num;
				label = dialog_event_get_selected_context(event);
				if (strcmp(label, okay_button_context) == 0)
				{
					if (dialog_event_get_popuplist_selected_indices(event, &response, &num) != BPS_SUCCESS)
					{
						BPSSetEffects(BPSGetEffects() & ~(EFF_TVLINES | EFF_SHOWFPS | EFF_SOFTEN));
					}
					else
					{
						int effects = BPSGetEffects();
						effects &= ~(EFF_TVLINES | EFF_SHOWFPS | EFF_SOFTEN);
						for (int i = 0; i < num; i++)
						{
							if (response[i] == 0)
								effects |= EFF_TVLINES;
							else if (response[i] == 1)
								effects |= EFF_SHOWFPS;
							else if (response[i] == 2)
								effects |= EFF_SOFTEN;
						}
						BPSSetEffects(effects);
						bps_free(response);
					}
				}
				break;
			}
		}
	}

	free(list);
	free(selected);
}

void ProgramInfo()
{
	dialog_instance_t alert_dialog = 0;
	dialog_request_events(0);
	if (dialog_create_alert(&alert_dialog) != BPS_SUCCESS)
		fprintf(stderr, "Failed to create alert dialog.");
	dialog_set_title_text(alert_dialog, "AlmostTI-PB 1.0");
	if (dialog_set_alert_html_message_text(alert_dialog, "<b>Original Source:</b> Marat Fayzullin<br><b>Website:</b> fms.komkon.org/ATI85/") != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to set alert dialog message text.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	const char* cancel_button_context = "Cancelled";

	const char* webpage_button_context = "WEB";
	dialog_add_button(alert_dialog, "komkon.org", true, webpage_button_context, true);
	if (dialog_add_button(alert_dialog, "Done", true, cancel_button_context, true) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to add button to alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	if (dialog_show(alert_dialog) != BPS_SUCCESS)
	{
		fprintf(stderr, "Failed to show alert dialog.");
		dialog_destroy(alert_dialog);
		alert_dialog = 0;
	}

	while(1)
	{
		bps_event_t* event = NULL;
		bps_get_event(&event, -1);

		if (event)
		{
			if (bps_event_get_domain(event) == dialog_get_domain())
			{
				const char* context = dialog_event_get_selected_context(event);
				if (strcmp(context, webpage_button_context) == 0)
					navigator_invoke("http://fms.komkon.org/ATI85", NULL);
				break;
			}
		}
	}
	if (alert_dialog)
		dialog_destroy(alert_dialog);
}

int doContextMenu(int &M)
{
	deviceinfo_details_t* data;
	deviceinfo_get_details(&data);
	const char* os = deviceinfo_details_get_device_os(data);

	if (strcmp(os, "BlackBerry 10") == 0)
	{
		deviceinfo_free_details(&data);
		return -1;
	}
	deviceinfo_free_details(&data);
	dialog_instance_t context;

	dialog_create_context_menu(&context);
	//dialog_set_context_menu_coordinates(context, 300, 1024);
	dialog_request_events(0);

	bool files = UpdateRomList(0);

	dialog_context_menu_add_button(context, "Load ROM", files, "LOAD", true, DIALOG_OPEN_LINK_ICON);
#ifdef DEBUG
	dialog_context_menu_add_button(context, "Toggle Effects", true, "EFFECTS", true, DIALOG_SELECT_ICON);
#endif
	dialog_context_menu_add_button(context, "ROM Info", true, "INFO", true, DIALOG_VIEW_IMAGE_ICON);
	dialog_context_menu_add_button(context, "About", true, "ABOUT", true, DIALOG_BOOKMARK_ICON);
	dialog_context_menu_add_button(context, "Cancel", true, "CANCEL", true, DIALOG_DELETE_ICON);

	dialog_show(context);
	int selected = -1;

	while(1)
	{
		bps_event_t* event = 0;
		bps_get_event(&event, -1);

		if (event)
		{
			if (bps_event_get_domain(event) == dialog_get_domain())
			{
				const char* ctxt = dialog_event_get_selected_context(event);
				if (strcmp(ctxt, "LOAD") == 0)
				{
					selected = 0;
				}
				else if (strcmp(ctxt, "ABOUT") == 0)
				{
					selected = 1;
				}
				else if (strcmp(ctxt, "EFFECTS") == 0)
				{
					selected = 2;
				}
				else if (strcmp(ctxt, "INFO") == 0)
				{
					selected = 3;
				}
				break;
			}
		}
	}
	if (context)
		dialog_destroy(context);

	switch (selected)
	{
	case 0:
		return AutoLoadRom(M);
	case 1:
		ProgramInfo();
		break;
	case 2:
		ToggleEffects();
		break;
	case 3:
		showRomInfo();
		break;
	}
	return -1;
}
