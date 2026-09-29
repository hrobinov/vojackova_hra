#pragma once

namespace agui
{
  /** For anything that blinks. Used by TextBox and TextField. */
  class BlinkingEvent
  {
  protected:
    ~BlinkingEvent() = default;
  public:
    BlinkingEvent() = default;

    /** Used to determine if isBlinking should return true.
     * Should be called in a widget's logic method.
     * @param elapsedTime The Amount of time the application has been running. */
    void processBlinkEvent(double elapsedTime);
    /** When this method returns true, a TextBox's caret is visible.
    * @return A boolean determining if the object should be seen. */
    bool isBlinking() const;
    /** Sets isBlinking to true and resets the amount of time before isBinking returns false.
     * When a delay between the next blink is needed, call this method. */
    void invalidateBlink();
    /** This will explicitly make isBlinking return the parameter boolean.
     * Should be called in a widget's logic method.
     * @param blinking The boolean isBlinking will return until the blink interval elapses. */
    void setBlinking(bool blinking);
    /** Determines how much time needs to elapse before isBlinking's return value changes.
     * Default is 0.5 (half of a second).
     * @param interval The time in seconds between each blink. */
    void setBlinkingInverval(double interval);

  private:
    double blinkInterval = 0.5;
    double lastBlinkTime = 0;
    bool blinking = true;
    bool blinkNeedsInvalidation = false;
  };
}
