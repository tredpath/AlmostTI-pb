/*
 * Menu.h
 *
 *  Created on: 2012-07-12
 *      Author: Travis Redpath
 */

#ifndef MENU_H_
#define MENU_H_

#include <sys/keycodes.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/navigator.h>
#include <bps/dialog.h>
#include <bps/event.h>
#include <string.h>
#include <string>
#include <vector>
#include <dirent.h>

#define OK_BUTTON  1
#define CANCEL_BUTTON  2

std::vector<std::string> sortAlpha(std::vector<std::string> sortThis);

int UpdateRomList(int allowRetry = 1);

int AutoLoadRom(int &M, bool showInfo = 0);

void ProgramInfo();

int doContextMenu(int &M);

int msg_box1(const char *title, const char *message);
int msg_box2(const char *title, const char *message);

#endif /* MENU_H_ */
