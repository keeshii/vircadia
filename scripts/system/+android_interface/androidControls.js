"use strict";
//
//  androidControls.js
//
//  Created by keeshii on September 26th, 2023.
//  Copyright 2022-2023 Overte e.V.
//
//  This script read touch screen events and triggers mouse events.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

(function () {

  Script.include("/~/system/libraries/controllerDispatcherUtils.js");
  var DISPATCHER_TOUCH_PROPERTIES = ["id", "position", "rotation", "dimensions", "registrationPoint"];

  var THROTTLE_TIME = 300;

  function AndroidControls() {
    this.onTouchStartFn = null;
    this.onTouchEndFn = null;
    this.clickedEntityId = null;
    this.touchStartTime = 0;
    this.touchEndTime = 0;
  }

  AndroidControls.prototype.intersectsOverlay = function (intersection) {
    if (intersection && intersection.intersects && intersection.overlayID) {
      return true;
    }
    return false;
  };

  AndroidControls.prototype.intersectsEntity = function (intersection) {
    if (intersection && intersection.intersects && intersection.entityID) {
      return true;
    }
    return false;
  };

  AndroidControls.prototype.findRayIntersection = function (pickRay) {
    // Check 3D overlays and entities. Argument is an object with origin and direction.
    var overlayRayIntersection = Overlays.findRayIntersection(pickRay);
    var entityRayIntersection = Entities.findRayIntersection(pickRay, true);
    var isOverlayInters = this.intersectsOverlay(overlayRayIntersection);
    var isEntityInters = this.intersectsEntity(entityRayIntersection);

    if (isOverlayInters && (!isEntityInters || overlayRayIntersection.distance < entityRayIntersection.distance)) {
      return {type: 'overlay', obj: overlayRayIntersection};
    } else if (isEntityInters) {
      return {type: 'entity', obj: entityRayIntersection};
    }
    return false;
  };

  AndroidControls.prototype.triggerEvent = function (eventType, event) {
    var info, entityId, pointerEvent, properties;
    info = this.findRayIntersection(Camera.computePickRay(event.x, event.y));

    if (!info && (eventType !== 'Release' || !this.clickedEntityId)) {
      return;
    }

    pointerEvent = {
      type: eventType,
      id: 1,
      pos2D: {x: 0, y: 0},
      pos3D: info ? info.obj.intersection : {x: 0, y: 0, z: 0},
      normal: info ? info.obj.surfaceNormal : {x: 0, y: 0, z: 0},
      direction: info ? info.obj.direction : {x: 0, y: 0, z: 0},
      button: "Primary",
      isPrimaryButton: true,
      isLeftButton: true,
      isPrimaryHeld: eventType === 'Press',
      isSecondaryHeld: false,
      isTertiaryHeld: false,
      keyboardModifiers: 0
    };

    if (!info) {
      entityId = this.clickedEntityId;
    } else if (info.type === "entity") {
      entityId = info.obj.entityID;
      properties = Entities.getEntityProperties(entityId, DISPATCHER_TOUCH_PROPERTIES);
      if (properties.id === entityId) {
        pointerEvent.pos2D = projectOntoEntityXYPlane(entityId, info.obj.intersection, properties);
      }
    } else {
      entityId = info.obj.overlayID;
      properties = Entities.getEntityProperties(entityId, DISPATCHER_TOUCH_PROPERTIES);
      if (properties.id === entityId) {
        pointerEvent.pos2D = projectOntoOverlayXYPlane(entityId, info.obj.intersection, properties);
      }
    }

    if (eventType === 'Press') {
      Entities.sendMousePressOnEntity(entityId, pointerEvent);
      Entities.sendClickDownOnEntity(entityId, pointerEvent);
      this.clickedEntityId = entityId;
    }

    if (eventType === 'Release') {
      if (info) {
        Entities.sendMouseReleaseOnEntity(entityId, pointerEvent);
      }
      if (this.clickedEntityId) {
        Entities.sendClickReleaseOnEntity(this.clickedEntityId, pointerEvent);
        this.clickedEntityId = null;
      }
    }
  };

  AndroidControls.prototype.onTouchStart = function (event) {
    var now = Date.now();
    if (this.touchStartTime + THROTTLE_TIME < now) {
      this.touchStartTime = now;
      this.triggerEvent('Press', event);
    }
  };

  AndroidControls.prototype.onTouchEnd = function (event) {
    var now = Date.now();
    if (this.touchEndTime + THROTTLE_TIME < now) {
      this.touchEndTime = now;
      this.triggerEvent('Release', event);
    }
  };

  AndroidControls.prototype.init = function () {
    var self = this;
    this.onTouchStartFn = function (ev) {
      self.onTouchStart(ev);
    };
    this.onTouchEndFn = function (ev) {
      self.onTouchEnd(ev);
    };

    Controller.touchBeginEvent.connect(this.onTouchStartFn);
    Controller.touchEndEvent.connect(this.onTouchEndFn);
  };

  AndroidControls.prototype.ending = function () {
    if (this.onTouchStartFn) {
      Controller.touchBeginEvent.disconnect(this.onTouchStartFn);
    }
    if (this.onTouchEndFn) {
      Controller.touchEndEvent.disconnect(this.onTouchEndFn);
    }
    this.clickedEntityId = null;
    this.touchStartTime = 0;
    this.touchEndTime = 0;
    this.onTouchStartFn = null;
    this.onTouchEndFn = null;
  };

  var androidControls = new AndroidControls();

  Script.scriptEnding.connect(function () {
    androidControls.ending();
  });
  androidControls.init();

  module.exports = androidControls;
}());
