//
//  Uploader.swift
//  BlueBasicConsole
//
//  Created by tim on 9/24/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

class Uploader: ConsoleDelegate {
  
  var okcount = 0
  let console: Console
  var complete: CompletionHandler? = nil
  var written = 0
  var wrote = 0
  
  init(_ console: Console) {
    self.console = console
  }
  
  func upload(url: NSURL, onComplete: CompletionHandler? = nil) {
    
    self.complete = onComplete

    console.delegate = self;
    
    console.status = "Sending...0%"
    
    var error = NSErrorPointer()
    var data = NSString(contentsOfURL: url, encoding: NSASCIIStringEncoding, error: error)

    write("NEW\n")
    for line in data!.componentsSeparatedByString("\n") as [String] {
      write(line + "\n")
    }
    write("END\n")
  }
  
  func write(str: String) {
    wrote += (str.utf16Count + 63) / 64
    console.write(str)
  }
  
  func onNotification(uuid: CBUUID, data: NSData) -> Bool {
    var str = NSString(data: data, encoding: NSASCIIStringEncoding)!
    if uuid == UUIDS.inputCharacteristicUUID && str == "OK\n" &&  ++okcount == 2 {
      console.status = "Connected"
      console.delegate = nil
      complete?(true)
      return true
    } else {
      return false
    }
  }
  
  func onWriteComplete(uuid: CBUUID) {
    written++
    console.status = String(format: "Sending...%d%%", 100 * written / wrote)
  }
}
