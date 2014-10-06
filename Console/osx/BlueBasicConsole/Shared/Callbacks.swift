//
//  Callbacks.swift
//  BlueBasicConsole
//
//  Created by tim on 9/23/14.
//  Copyright (c) 2014 tim. All rights reserved.
//

import Foundation
import CoreBluetooth

typealias CompletionHandler = Bool -> Void
typealias NewDeviceFoundHandler = Device -> Void
typealias ServicesFoundHandler = [CBUUID: DeviceService] -> Void
typealias CharacteristicUpdateHandler = NSData -> Void

class Callback<T> {
  
  let callbacks: Callbacks<T>
  let id: Int
  
  init(callbacks: Callbacks<T>, id: Int) {
    self.callbacks = callbacks
    self.id = id
  }

  func remove() {
    callbacks.remove(id)
  }
  
}


class Callbacks<T> {
  
  var callbacks = Array<(T -> Void)?>()
  
  func append(callback: (T -> Void)?) -> Callback<T> {
    callbacks.append(callback)
    return Callback<T>(callbacks: self, id: callbacks.count - 1)
  }
  
  func remove(id: Int) {
    callbacks.removeAtIndex(id)
  }
  
  func removeAll() {
    callbacks.removeAll(keepCapacity: false)
  }
  
  func call(arg: T) {
    for callback in callbacks {
      callback?(arg)
    }
  }

}

class OneTimeCallbacks<T> : Callbacks<T> {
  
  override func call(arg: T) {
    var ocallbacks = callbacks
    callbacks.removeAll(keepCapacity: false)
    for callback in ocallbacks {
      callback?(arg)
    }
  }
  
}
