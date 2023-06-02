"use strict";

(function() {

  var TAP_DELAY = 300;

  function AndroidControls() {
    this.onTouchStartFn = null;
    this.onTouchEndFn = null;
    this.touchStartTime = 0;
  }

  AndroidControls.prototype.intersectsOverlay = function(intersection) {
      if (intersection && intersection.intersects && intersection.overlayID) {
          return true;
      }
      return false;
  }

  AndroidControls.prototype.intersectsEntity = function(intersection) {
      if (intersection && intersection.intersects && intersection.entityID) {
          return true;
      }
      return false;
  }

  AndroidControls.prototype.findRayIntersection = function(pickRay) {
      // Check 3D overlays and entities. Argument is an object with origin and direction.
      var overlayRayIntersection = Overlays.findRayIntersection(pickRay);
      var entityRayIntersection = Entities.findRayIntersection(pickRay, true);
      var isOverlayInters = this.intersectsOverlay(overlayRayIntersection);
      var isEntityInters = this.intersectsEntity(entityRayIntersection);
      
      if (isOverlayInters && (!isEntityInters || overlayRayIntersection.distance < entityRayIntersection.distance)) {
        return { type: 'overlay', obj: overlayRayIntersection };
      } else if (isEntityInters) {
        return { type: 'entity', obj: entityRayIntersection };
      }
      return false;
  }

  AndroidControls.prototype.triggerClick = function (event) {
    var intersection, entityId, pointerEvent;
    intersection = this.findRayIntersection(Camera.computePickRay(event.x, event.y));
    if (intersection) {
        entityId = intersection.type === "entity"
          ? intersection.obj.entityID
          : intersection.obj.overlayID;
        pointerEvent = {
          type: "Press",
          id: 1,
          pos2D: { x: event.x, y: event.y },
          pos3D: intersection.origin,
          direction: intersection.direction,
          button: "Primary",
          isPrimaryButton: true,
          isLeftButton: true,
          keyboardModifiers: 0
        };
        Entities.sendMousePressOnEntity(entityId, pointerEvent);
        Entities.sendClickDownOnEntity(entityId, pointerEvent);
    }
  };

  AndroidControls.prototype.onTouchStart = function (_event) {
    this.touchStartTime = Date.now();
  };

  AndroidControls.prototype.onTouchEnd = function (event) {
    var now = Date.now();
    if (now - this.touchStartTime < TAP_DELAY) {
      this.triggerClick(event);
    }
    this.touchStartTime = 0;
  };

  AndroidControls.prototype.init = function () {
    var self = this;
    this.onTouchStartFn = function (ev) { self.onTouchStart(ev); };
    this.onTouchEndFn = function (ev) { self.onTouchEnd(ev); };

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
    this.touchStartTime = 0;
    this.onTouchStartFn = null;
    this.onTouchEndFn = null;
  };

  var androidControls = new AndroidControls();

  Script.scriptEnding.connect(function () { androidControls.ending(); });
  androidControls.init();

  module.exports = androidControls;
}());
