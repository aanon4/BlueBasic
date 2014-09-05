//
//  AppDelegate.m
//  BlueBasicConsole
//
//  Created by tim on 7/28/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

@synthesize window;
@synthesize watcher;
@synthesize console;
@synthesize pending;
@synthesize status;
@synthesize devices;
@synthesize loadingOrSaving;
@synthesize okcount;


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  watcher = [[BLEConsoleWatcher alloc] init];
  watcher.delegate = self;
  pending = [NSMutableString stringWithCapacity:100];
  [console setAutomaticQuoteSubstitutionEnabled:FALSE];
  [console setFont:[NSFont fontWithName:@"Menlo Regular" size:11]];
  [devices setDataSource:watcher];
  [devices setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleRegular];
  [devices setDoubleAction:@selector(selectDevice:)];
  loadingOrSaving = FALSE;
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
  [watcher stop];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
  return YES;
}

- (void)didReceive:(NSString *)data
{
  if (loadingOrSaving)
  {
    if ([data isEqualToString:@"OK\n"])
    {
      okcount++;
    }
  }
  else
  {
    [console replaceCharactersInRange:NSMakeRange([[console string] length], 0) withString:data];
    [console scrollRangeToVisible:NSMakeRange([[console string] length], 0)];
    [console setNeedsDisplay:YES];
  }
}

- (BOOL)textView:(NSTextView *)textView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString
{
  if (loadingOrSaving)
  {
    return NO;
  }

  unsigned long tlen = [textView string].length;
  unsigned long slen = replacementString.length;
  
  if (slen > 0)
  {
    for (int idx = 0; idx < slen; idx++)
    {
      NSString* ch = [replacementString substringWithRange:NSMakeRange(idx, 1)];
      [pending appendString:ch];
      if ([ch isEqualToString:@"\n"])
      {
        [watcher send:pending];
        pending = [NSMutableString stringWithCapacity:100];
      }
    }
    if (affectedCharRange.location == tlen)
    {
      return YES;
    }
    else
    {
      [textView replaceCharactersInRange:NSMakeRange(tlen, 0) withString:replacementString];
      [textView setSelectedRange:NSMakeRange([textView string].length, 0)];
      return NO;
    }
  }
  else if (affectedCharRange.location == tlen - 1 && pending.length > 0)
  {
    [pending deleteCharactersInRange:NSMakeRange(pending.length - 1, 1)];
    return YES;
  }
  else
  {
    return NO;
  }
}

- (void)updateStatus:(NSString *)message
{
  [status setStringValue:message];
  if ([message isEqualToString:@"Connected"])
  {
    [console setEditable:TRUE];
    [window makeFirstResponder:console];
  }
  else
  {
    [console setEditable:FALSE];
  }
}

- (void)foundNewPeripheral:(CBPeripheral *)peripheral
{
  [devices reloadData];
}

- (void)selectDevice:(id)object
{
  [watcher connectTo:[devices clickedRow]];
}

- (IBAction)toDevice:(id)selector
{
  NSOpenPanel* dialog = [NSOpenPanel openPanel];
  [dialog setCanChooseFiles:YES];
  [dialog setTitle:@"Select file to load onto device"];
  if ([dialog runModal] == NSOKButton)
  {
    NSArray* urls = [dialog URLs];
    if ([urls count] > 0)
    {
      [self updateStatus:@"Sending..."];
      watcher.writeCount = 1;
      watcher.writtenCount = 1;
      okcount = 0;
      loadingOrSaving = TRUE;
      NSString* data = [NSString stringWithContentsOfURL:urls[0] encoding:NSUTF8StringEncoding error:NULL];
      [watcher send:@"NEW\n"];
      for (NSString* line in [data componentsSeparatedByString:@"\n"])
      {
        [watcher send:line];
        [watcher send:@"\n"];
      }
      [watcher send:@"SAVE\n"];
      BOOL running = YES;
      while (okcount < 2 && running)
      {
        running = [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        [self updateStatus:[NSString stringWithFormat:@"Sending...%d%%", 100 * watcher.writtenCount / watcher.writeCount]];
      }
      loadingOrSaving = FALSE;
      [self updateStatus:@"Connected"];
    }
  }
}

- (IBAction)fromDevice:(id)selector
{
}

- (IBAction)updateFirmware:(id)selector
{
  NSOpenPanel* dialog = [NSOpenPanel openPanel];
  [dialog setCanChooseFiles:YES];
  [dialog setTitle:@"Select new firmware for device"];
  if ([dialog runModal] == NSOKButton)
  {
    NSArray* urls = [dialog URLs];
    if ([urls count] > 0)
    {
      // If status is Connected we need to force the device into upgrade mode
      if ([[status stringValue] isEqualToString:@"Connected"])
      {
        [watcher send:@"REBOOT UP\n"];
        BOOL running = YES;
        while (running && !([[status stringValue] isEqualToString:@"Upgradable"]))
        {
          running = [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        }
      }
      [watcher upgrade:[NSData dataWithContentsOfURL:urls[0]]];
    }
  }
}

@end
