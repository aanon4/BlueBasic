//
//  AppDelegate.h
//  BlueBasicConsole
//
//  Created by tim on 7/28/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BLEConsoleWatcher.h"


@interface AppDelegate : NSObject <NSApplicationDelegate, BLEConsoleDelegate, NSTextViewDelegate>

@property (assign) IBOutlet NSWindow *window;
@property BLEConsoleWatcher* watcher;
@property IBOutlet NSTextView* console;
@property IBOutlet NSTextFieldCell* status;
@property IBOutlet NSTableView* devices;
@property NSMutableString* pending;
@property BOOL loadingOrSaving;
@property int okcount;

- (IBAction)toDevice:(id)selector;
- (IBAction)fromDevice:(id)selector;
- (IBAction)updateFirmware:(id)selector;

@end
