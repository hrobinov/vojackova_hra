#pragma once

namespace agui
{
  /** Interface for changing the cursor.
   * Must be implemented by a back end. */
  class CursorProvider
  {
  public:
    enum CursorEnum
    {
      DEFAULT_CURSOR,
      ARROW_CURSOR,
      BUSY_CURSOR,
      QUESTION_CURSOR,
      EDIT_CURSOR,
      MOVE_CURSOR,
      RESIZE_N_CURSOR,
      RESIZE_W_CURSOR,
      RESIZE_S_CURSOR,
      RESIZE_E_CURSOR,
      RESIZE_NW_CURSOR,
      RESIZE_SW_CURSOR,
      RESIZE_SE_CURSOR,
      RESIZE_NE_CURSOR,
      LINK_CURSOR,
    };

    /** Attempts to set the cursor to the requested cursor.
     * @return True if the cursor was changed. */
    virtual bool setCursor(CursorEnum cursor) = 0;
    CursorProvider() {}
    virtual ~CursorProvider() {}
  };
}
