import 'package:flutter/material.dart';
import 'dart:async';

class MainWidget extends StatefulWidget {
  @override
  State createState() => new MainWidgetState();
}

class MainWidgetState extends State<MainWidget> {
  List<ItemData> _data = new List();
  Timer timer;

  Widget build(BuildContext context) {
    return new ListView(
        children: _data.map((item) => new ChildWidget(item)).toList());
  }

  @override
  void initState() {
    super.initState();
    timer = new Timer.periodic(new Duration(seconds: 1), (Timer timer) async {
      ItemData data = await loadData();
      this.setState(() {
        _data = <ItemData>[data];
      });
    });
  }

  @override
  void dispose() {
    super.dispose();
    timer.cancel();
  }

  static int testCount = 0;

  Future<ItemData> loadData() async {
    testCount++;
    return new ItemData("Testing #$testCount");
  }
}

class ChildWidget extends StatefulWidget {
  ItemData _data;

  ChildWidget(ItemData data) {
    _data = data;
  }

  @override
  State<ChildWidget> createState() => new ChildState();
}

class ChildState extends State<ChildWidget> {
  @override
  Widget build(BuildContext context) {
    return new GestureDetector(
        onTap: () => foo(),
        child: new Padding(
          padding: const EdgeInsets.symmetric(vertical: 12.0, horizontal: 24.0),
          child: new Card(
            child: new Container(
              padding: const EdgeInsets.all(8.0),
              child: new Text(widget._data.title),
            ),
          ),
        ));
  }

  foo() {
    print("Card Tapped: " + widget._data.toString());
  }
}

class ItemData {
  final String title;

  ItemData(this.title);

  @override
  String toString() {
    return 'ItemData{title: $title}';
  }
}
