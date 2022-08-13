<h1 align="center">
    Tui Widgets
</h1>


<h3 align="center">
  <a href="https://tuiwidgets.namepad.de/latest/">Documentation</a>
  <span> · </span>
  <a href="https://tuiwidgets.namepad.de/latest/getting-started.html">Getting Started</a>
  <span> · </span>
</h3>

High level terminal user interface library for modern terminals.


## Documentation

The full documentation for Tui Widgets can be found [here](https://tuiwidgets.namepad.de/latest/).

## Building / Installing

See https://tuiwidgets.namepad.de/latest/building-tuiwidgets.html

## Example

See [Getting started](https://tuiwidgets.namepad.de/latest/getting-started.html)
or [full header](doc/examples/getting-started/getting-started.h)
and [full implementation](doc/examples/getting-started/getting-started.cpp).

```
Tui::ZWindow *win = new Tui::ZWindow("Hello World", this);
win->setGeometry({5, 3, 20, 10});
QObject::connect(new Tui::ZShortcut(Tui::ZKeySequence::forKey(Qt::Key_Escape),
                                    this,
                                    Qt::ApplicationShortcut),
                 &Tui::ZShortcut::activated,
                 this, &Main::quit);
Tui::ZButton *button = new Tui::ZButton(Tui::withMarkup, "<m>Q</m>uit", win);
QObject::connect(button, &Tui::ZButton::clicked, this, &Main::quit);
button->setGeometry({6, 7, 10, 1});
button->setFocus();
```

## License

Tui Widgets is licensed under the [Boost Software License 1.0](COPYING)
