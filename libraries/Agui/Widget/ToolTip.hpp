#pragma once
#include "Agui/Widget/Window.hpp"
#include "Agui/ToolTipCreatorBase.hpp"

namespace agui
{
  class ToolTip : public agui::Window
  {
    using super = agui::Window;
  public:
    ToolTip(GuiDirection direction, const agui::FrameStyle* parentStyle = nullptr);
    ToolTip(const agui::FrameStyle* parentStyle = nullptr);
    ToolTip(std::string caption, std::string text);
    virtual void updateContent();
    void updatePosition(const agui::Point& mousePosition);
    void centerOnMouse();
    virtual void resizeToContents() override;
    virtual bool isValid() const { return true; }
    virtual bool doTooltipLogic() const { return true; } // bring to front and center on mouse each frame
    virtual bool isToolTip() const final override { return true; }
  protected:
    agui::Label* addLabel(const std::string& content, agui::LabelStyle* style, std::optional<agui::Color> colorOverride = std::nullopt);
  public:
    static FrameStyle defaultToolTipStyle;
    static FrameStyle defaultToolTipOuterStyle;
    static LabelStyle defaultTitleStyle;
    static LabelStyle defaultLabelStyle;

    bool activatedByInstantToolTip = false;
    bool isEntityToolTip = false;
    std::string caption;
    std::string text;
    std::optional<agui::Color> captionColor;
  };

  class ToolTipWithCallback : public agui::ToolTip
  {
  public:
    using Callback = std::function<void(bool)>;
    ToolTipWithCallback(std::string caption, std::string text, const Callback& callback)
      : agui::ToolTip(std::move(caption), std::move(text))
      , callback(callback)
    { if (this->callback) this->callback(true); }
    ~ToolTipWithCallback() { if (this->callback) this->callback(false); }

    Callback callback;
  };

  class PlainToolTipCreator : public agui::ToolTipCreatorBase
  {
  public:
    template<class T>
    PlainToolTipCreator(T&& title)
      : title(std::forward<T>(title))
    {}

    template<class T1, class T2>
    PlainToolTipCreator(T1&& title, T2&& text)
      : title(std::forward<T1>(title))
      , text(std::forward<T2>(text))
    {}

    virtual agui::ToolTip* createToolTip() override
    {
      return this->title.empty()
             ? new ToolTip(this->text, std::string())
             : new ToolTip(this->title, this->text);
    }

    const std::string& getTitle() const { return this->title; }
    const std::string& getText() const { return this->text; }
    void appendTitle(std::string&& title) { this->title += std::move(title); }

  protected:
    std::string title;
    std::string text;
  };

  class PlainToolTipCreatorWithCallback : public agui::PlainToolTipCreator
  {
  public:
    template<class T>
    PlainToolTipCreatorWithCallback(T&& title, ToolTipWithCallback::Callback callback)
      : PlainToolTipCreator(std::forward<T>(title))
      , callback(std::move(callback))
    {}

    template<class T1, class T2>
    PlainToolTipCreatorWithCallback(T1&& title, T2&& text, ToolTipWithCallback::Callback callback)
      : PlainToolTipCreator(std::forward<T1>(title), std::forward<T1>(text))
      , callback(std::move(callback))
    {}

    virtual agui::ToolTip* createToolTip() override
    {
      return this->title.empty()
             ? new ToolTipWithCallback(this->text, std::string(), this->callback)
             : new ToolTipWithCallback(this->title, this->text, this->callback);
    }

    ToolTipWithCallback::Callback callback;
  };

  class ToolTipCreatedByCallback : public agui::ToolTipCreatorBase
  {
  public:
    using Callback = std::function<ToolTip*()>;
    ToolTipCreatedByCallback(Callback&& callback)
      : callback(callback)
    {}

    virtual agui::ToolTip* createToolTip() override
    {
      return this->callback();
    }

    Callback callback;
  };
}
