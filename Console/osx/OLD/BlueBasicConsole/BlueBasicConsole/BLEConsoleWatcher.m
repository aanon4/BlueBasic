//
//  BLEConsoleWatcher.m
//  BlueBasicConsole
//
//  Created by tim on 7/28/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

#import "BLEConsoleWatcher.h"

@implementation BLEConsoleWatcher

@synthesize delegate;
@synthesize manager;
@synthesize peripherals;
@synthesize commsServiceUUID;
@synthesize inputCharacteristicUUID;
@synthesize inputCharacteristic;
@synthesize outputCharacteristicUUID;
@synthesize outputCharacteristic;
@synthesize oadServiceUUID;
@synthesize oadImgIdentityUUID;
@synthesize oadImgBlockUUID;
@synthesize imgIdentityCharacteristic;
@synthesize imgBlockCharacteristic;
@synthesize connected;
@synthesize writeCount;
@synthesize writtenCount;
@synthesize imgCount;


- (BLEConsoleWatcher*)init
{
  self = [super init];

  commsServiceUUID = [CBUUID UUIDWithString:@"25FB9E91-1616-448D-B5A3-F70A64BDA73A"];
  inputCharacteristicUUID = [CBUUID UUIDWithString:@"C3FBC9E2-676B-9FB5-3749-2F471DCF07B2"];
  outputCharacteristicUUID = [CBUUID UUIDWithString:@"D6AF9B3C-FE92-1CB2-F74B-7AFB7DE57E6D"];
  oadServiceUUID = [CBUUID UUIDWithString:@"F000FFC0-0451-4000-B000-000000000000"];
  oadImgBlockUUID = [CBUUID UUIDWithString:@"F000FFC2-0451-4000-B000-000000000000"];
  oadImgIdentityUUID = [CBUUID UUIDWithString:@"F000FFC1-0451-4000-B000-000000000000"];
  
  peripherals = [NSMutableArray arrayWithCapacity:10];
  
  manager = [[CBCentralManager alloc] initWithDelegate:self queue:dispatch_get_main_queue()];

  return self;
}

- (void)stop
{
  CBPeripheral* old = connected;
  if (old)
  {
    connected = nil;
    [manager cancelPeripheralConnection:old];
  }
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
  //NSLog(@"Central state: %d\n", central.state);
  switch (central.state)
  {
    case CBCentralManagerStatePoweredOn:
      [self updateStatus:@"Not connected"];
      [manager scanForPeripheralsWithServices:nil options:nil];
      break;
    default:
      break;
  }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
  if ([peripherals indexOfObject:peripheral] == NSNotFound)
  {
    //NSLog(@"Found: %@\n", peripheral.name);
    [peripherals addObject:peripheral];
    if (delegate)
    {
      [delegate foundNewPeripheral:peripheral];
    }
  }
}

- (void)connectTo:(NSUInteger)deviceIndex
{
  CBPeripheral* old = connected;
  connected = peripherals[deviceIndex];
  if (old)
  {
    [manager cancelPeripheralConnection:old];
  }
  [manager connectPeripheral:connected options:nil];
  [self updateStatus:@"Connecting..."];
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
  //NSLog(@"Connected\n");
  peripheral.delegate = self;
  //[peripheral discoverServices:[NSArray arrayWithObjects:commsServiceUUID, oadServiceUUID, nil]];
  [peripheral discoverServices:[NSArray array]];
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral error:(NSError *)error
{
  //NSLog(@"Disconnect %@ %@ - retry\n", peripheral, error);
  if (connected == peripheral)
  {
    [manager connectPeripheral:connected options:nil];
  }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
  //NSLog(@"Services: %@ %@\n", peripheral.services, error);
  if (peripheral == connected)
  {
    if (!peripheral.services)
    {
      [manager cancelPeripheralConnection:peripheral];
      //NSLog(@"Failed - Retry\n");
      if (connected == peripheral)
      {
        [manager connectPeripheral:peripheral options:nil];
      }
    }
    else
    {
      bool found = false;
      for (CBService* svc in peripheral.services)
      {
        //NSLog(@"%@\n", svc.UUID);
        if ([svc.UUID isEqual:commsServiceUUID] || [svc.UUID isEqual:oadServiceUUID])
        {
          [peripheral discoverCharacteristics:[NSArray array] forService:svc];
          found = true;
          break;
        }
      }
      if (!found)
      {
        connected = nil;
        [self updateStatus:@"Failed"];
        [manager cancelPeripheralConnection:peripheral];
      }
    }
  }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
  if (peripheral == connected)
  {
    int count = 0;
    int fcount = 0;
    //NSLog(@"Characteristics: %@ %@\n", service.characteristics, error);
    for (CBCharacteristic* ch in service.characteristics)
    {
      //NSLog(@"%@\n", ch.UUID);
      if ([ch.UUID isEqual:inputCharacteristicUUID])
      {
        inputCharacteristic = ch;
        [peripheral setNotifyValue:YES forCharacteristic:ch];
        //NSLog(@"Connected read\n");
        count++;
      }
      else if ([ch.UUID isEqual:outputCharacteristicUUID])
      {
        outputCharacteristic = ch;
        //NSLog(@"Connected write\n");
        count++;
      }
      else if ([ch.UUID isEqual:oadImgBlockUUID])
      {
        imgBlockCharacteristic = ch;
        fcount++;
      }
      else if ([ch.UUID isEqual:oadImgIdentityUUID])
      {
        imgIdentityCharacteristic = ch;
        fcount++;
      }
    }
    if (count == 2)
    {
      [self updateStatus:@"Connected"];
    }
    else if (fcount == 2)
    {
      [self updateStatus:@"Upgradable"];
    }
    else
    {
      connected = nil;
      [self updateStatus:@"Failed"];
      [manager cancelPeripheralConnection:peripheral];
    }
  }
}

- (void)send:(NSString*)data
{
  if (connected && outputCharacteristic)
  {
    if (data.length > 64)
    {
      for (int offset = 0; offset < data.length; offset += 64)
      {
        NSString* piece = [data substringWithRange:NSMakeRange(offset, MIN(64, data.length - offset))];
        writeCount++;
        [connected writeValue:[piece dataUsingEncoding:NSASCIIStringEncoding] forCharacteristic:outputCharacteristic type:CBCharacteristicWriteWithResponse];
      }
    }
    else
    {
      writeCount++;
      [connected writeValue:[data dataUsingEncoding:NSASCIIStringEncoding] forCharacteristic:outputCharacteristic type:CBCharacteristicWriteWithResponse];
    }
  }
}

- (void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
  if ([characteristic isEqual:outputCharacteristic])
  {
    writtenCount++;
  }
  else if ([characteristic isEqual:imgIdentityCharacteristic] || [characteristic isEqual:imgBlockCharacteristic])
  {
    imgCount++;
  }
}


- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *)error
{
  //NSLog(@"value: %@ %@\n", characteristic.value, error);
  if (delegate)
  {
    [delegate didReceive:[[NSString alloc] initWithData:characteristic.value encoding:NSASCIIStringEncoding]];
  }
}

- (void)updateStatus:(NSString*)status
{
  if (delegate)
  {
    [delegate updateStatus:status];
  }
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
  return peripherals.count;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
  CBPeripheral* p = (CBPeripheral*)peripherals[row];
  if (p.name == NULL)
  {
    return @"(null)";
  }
  else
  {
    return p.name;
  }
}

- (void)upgrade:(NSData*)firmware
{
  const int BLOCKSIZE = 16;
  
  // Initiate the upgrade
  unsigned char* bytes = (unsigned char*)[firmware bytes];
  unsigned char init[8] =
  {
    bytes[4], bytes[5], // version
    bytes[6], bytes[7], // length
    bytes[8], bytes[9], bytes[10], bytes[11] // uid
  };

  imgCount = 0;
  [connected writeValue:[NSData dataWithBytes:init length:sizeof(init)] forCharacteristic:imgIdentityCharacteristic type:CBCharacteristicWriteWithResponse];
  BOOL running = YES;
  while (imgCount == 0 && running)
  {
    running = [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
  }

  // Write all the blocks
  imgCount = 0;
  int imgWrite = 0;
  unsigned long len = [firmware length];
  for (unsigned long i = 0; i < len; i += BLOCKSIZE)
  {
    NSMutableData* block = [NSMutableData dataWithCapacity:BLOCKSIZE + 2];
    unsigned long blocknr = i / BLOCKSIZE;
    unsigned char blk[2] = { blocknr, blocknr >> 8 };
    [block appendBytes:blk length:sizeof(blk)];
    [block appendData:[firmware subdataWithRange:NSMakeRange(i, BLOCKSIZE)]];
    [connected writeValue:block forCharacteristic:imgBlockCharacteristic type:CBCharacteristicWriteWithResponse];
    imgWrite++;
  }

  // Wait for them to be done
  running = YES;
  imgWrite--; // The last block reboots the device, so don't expect a reply
  while (imgCount < imgWrite && running)
  {
    running = [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
    [self updateStatus:[NSString stringWithFormat:@"Upgrading...%.1f%%", 100.0 * imgCount / imgWrite]];
  }
  [self updateStatus:@"Waiting..."];
  
  // But we do need to pause a little before disconnecting otherwise the last packet won't be sent
  [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:5]];

  // Disconnect (and autoreconnect)
  [manager cancelPeripheralConnection:connected];
}

@end
