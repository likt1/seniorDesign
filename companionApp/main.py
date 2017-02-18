from kivy.app import App
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.properties import NumericProperty
from kivy.lang import Builder

screens = ["Wifi","Dropbox","Inspirado"]
Builder.load_string('''
#:import random random.random
#:import SlideTransition kivy.uix.screenmanager.SlideTransition
<HomeScreen>:
    hue: .5
    canvas:
        Color:
            hsv: self.hue, .5, .3
        Rectangle:
            size: self.size
    Label:
        font_size: 42
        text: root.name
    Button:
        text: 'Dropbox'
        size_hint: None, None
        pos_hint: {'right': 1, 'top': 0.9}
        size: 150, 50
        on_release: root.manager.current = 'DropboxSettings'
    Button:
        text: 'Wifi'
        size_hint: None, None
        pos_hint: {'center_x': .5, 'top': 0.9}
        size: 150, 50
        on_release: root.manager.current = 'WifiSettings'
    Button:
        text: 'Inspirado'
        size_hint: None, None
        pos_hint: {'left': 1, 'top': 0.9}
        size: 150, 50
        on_release: root.manager.current = 'InspiradoSettings'

<DropboxScreen>:
    BoxLayout:
        orientation: 'vertical'
        Button:
            text: 'Inspirado Settings'
        Button:
            text: 'Back to menu'
            on_press: root.manager.current = 'Home'

<WifiScreen>: 
    BoxLayout:
        orientation: 'vertical'
        Button:
            text: 'Wifi Settings'
        Button:
            text: 'Back to menu'
            on_press: root.manager.current = 'Home'

<InspiradoScreen>:
    BoxLayout:
        orientation: 'vertical'
        Button:
            text: 'Inspirado Settings'
        Button:
            text: 'Back to menu'
            on_press: root.manager.current = 'Home'
''')


class HomeScreen(Screen):
    hue = NumericProperty(0)

class DropboxScreen(Screen):
    pass

class InspiradoScreen(Screen):
    pass

class WifiScreen(Screen):
    pass


class ScreenManagerApp(App):
    def build(self):
        root = ScreenManager()
        root.add_widget(HomeScreen(name="Home"))
        root.add_widget(DropboxScreen(name='DropboxSettings'))
        root.add_widget(WifiScreen(name='WifiSettings'))
        root.add_widget(InspiradoScreen(name='InspiradoSettings'))
        return root

if __name__ == '__main__':
    ScreenManagerApp().run()
