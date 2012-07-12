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

std::vector<std::string> sortAlpha(std::vector<std::string> sortThis);

void UpdateRomList();

int AutoLoadRom(int &M, bool showAbout = 0);

void ProgramInfo();

#endif /* MENU_H_ */
