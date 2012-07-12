/*
 * QNXMenu.cpp
 *
 *  Created on: 2012-07-12
 *      Author: Travis Redpath
 */

#include "QNXMenu.h"
#include "../TI85.h"

#include <stdlib.h>

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
		for (size_t count = 1; count < sortThis.size() - 1; count++)
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

void UpdateRomList()
{
	DIR* dirp;
	struct dirent* direntp;

	vsList.clear();
	sortedvecList.clear();
	vsList.push_back(Title);

	dirp = opendir("shared/misc/TI/ROM/");
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
				if (tmp.compare(Config[i].ROMFile) == 0)
					break;
			if (Config[i].ROMFile)
				vsList.push_back(direntp->d_name);
		}
	}
	else
		fprintf(stderr, "dirp is NULL ...\n");
	fprintf(stderr, "number of files %d\n", vsList.size());

	if (vsList.size() == 1)
	{
		dialog_instance_t alert_dialog = 0;
		dialog_request_events(0);
		if (dialog_create_alert(&alert_dialog) != BPS_SUCCESS)
			fprintf(stderr, "Failed to create alert dialog.");
		dialog_set_title_text(alert_dialog, "Almost-TI Error Report");
		if (dialog_set_alert_html_message_text(alert_dialog, "<u><b>ERROR:</b> You do not have any ROMS!<br></u><b>Add TI ROMS to:</b> <br><u>\"shared/misc/TI/ROM\"</u><br>using either <i>WiFi sharing</i> or <i>Desktop Software.</i>") != BPS_SUCCESS)
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
				{
					int selectedIndex = dialog_event_get_selected_index(event);
					const char* label = dialog_event_get_selected_label(event);
					const char* context = dialog_event_get_selected_context(event);
					exit(-1);
				}
			}
		}
		if (alert_dialog)
			dialog_destroy(alert_dialog);
	}

	sortedvecList = sortAlpha(vsList);
}

int AutoLoadRom(int &M, bool showAbout)
{
	int status = 0;
	M = -1;

	const char** list = 0;
	size_t count = 0;
	if (showAbout)
		list = (const char**)malloc((sortedvecList.size() + 2) * sizeof(char*));
	else
		list = (const char**)malloc(sortedvecList.size() * sizeof(char*));

	for(;;)
	{
		if (count >= sortedvecList.size())
			break;
		fprintf(stderr, "%d \n", count);
		list[count] = sortedvecList[count].c_str();
		count++;
	}
	if (showAbout)
	{
		list[count++] = "Other";
		list[count++] = "About";
	}

	dialog_instance_t dialog = 0;
	int i, rc;
	bps_event_t *event;
	int domain = 0;
	const char* label;
	char romfilename[256];
	dialog_create_popuplist(&dialog);
	dialog_set_popuplist_items(dialog, list, count);
	if (showAbout)
	{
		int indice[] = { 0, sortedvecList.size() };
		dialog_set_popuplist_separator_indices(dialog, (int*)&indice, 2);
	}
	else
	{
		int indice[] = { 0 };
		dialog_set_popuplist_separator_indices(dialog, (int*)&indice, 1);
	}

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
							strcpy(romfilename, list[*response[0]]);
						}
						else
						{
							ProgramInfo();
							free(list);
							return -1;
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

	return WhichTI(romfilename, M);
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
			{
				int selectedIndex = dialog_event_get_selected_index(event);
				const char* label = dialog_event_get_selected_label(event);
				const char* context = dialog_event_get_selected_context(event);
				break;
			}
		}
	}
	if (alert_dialog)
		dialog_destroy(alert_dialog);
}
