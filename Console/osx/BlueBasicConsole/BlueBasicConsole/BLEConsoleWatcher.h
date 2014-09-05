//
//  BLEConsoleWatcher.h
//  BlueBasicConsole
//
//  Created by tim on 7/28/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@protocol BLEConsoleDelegate <NSObject>

- (void)didReceive:(NSString*) data;
- (void)updateStatus:(NSString*) status;
- (void)foundNewPeripheral:(CBPeripheral*) peripheral;

@end

@interface BLEConsoleWatcher : NSObject <CBCentralManagerDelegate, CBPeripheralDelegate, NSTableViewDataSource>

- (BLEConsoleWatcher*)init;
- (void)stop;
- (void)send:(NSString*) data;
- (void)connectTo:(NSUInteger)deviceIndex;
- (void)upgrade:(NSData*)firmware;

@property id<BLEConsoleDelegate> delegate;
@property CBCentralManager* manager;
@property NSMutableArray* peripherals;
@property CBUUID* commsServiceUUID;
@property CBUUID* inputCharacteristicUUID;
@property CBUUID* outputCharacteristicUUID;
@property CBUUID* oadServiceUUID;
@property CBUUID* oadImgIdentityUUID;
@property CBUUID* oadImgBlockUUID;
@property CBPeripheral* connected;
@property CBCharacteristic* inputCharacteristic;
@property CBCharacteristic* outputCharacteristic;
@property CBCharacteristic* imgIdentityCharacteristic;
@property CBCharacteristic* imgBlockCharacteristic;
@property int writeCount;
@property int writtenCount;
@property int imgCount;

@end

