import 'package:flutter/material.dart';
import 'announce.dart';

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key, this.title}) : super(key: key) {
    print(
        ">>>>>>>>>>>>>>>>>>>>>>>>>   MyHomePage being created. Constructor called.");
  }
  final String title;
  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  // constructor
  _MyHomePageState() : super() {
    print(
        ">>>>>>>>>>>>>>>>>>>>>>>>>   _MyHomePageState being created. Constructor called.");
  }
  List<Widget> m_list = new List();

  Widget buildContext(BuildContext ctxt, int index) {
    return Card(
      child: m_list[index],
    );
  }

  @override
  Widget build(BuildContext context) {
    print(">>>>>>>>>>>>>>>>>>>>>>>>>   _MyHomePageState state build");
    return Scaffold(
      body: ListView.builder(
        shrinkWrap: true,
        itemCount: m_list.length,
        itemBuilder: (BuildContext ctxt, int index) =>
            buildContext(ctxt, index),
      ),
      floatingActionButton: Container(
        child: Row(
          mainAxisAlignment: MainAxisAlignment.end,
          children: [
            FloatingActionButton(
              onPressed: () {
                setState(() {
                  m_list.add(Announce.announceSection);
                  print(">>>>>>>>>>>>>>>>>>>>>>>>>   ADD Button pressed");
                });
              },
              tooltip: 'Increment',
              child: Icon(Icons.add),
            ),
            FloatingActionButton(
              onPressed: () {
                setState(() {
                  m_list.removeLast();
                  print(">>>>>>>>>>>>>>>>>>>>>>>>>   REMOVE Button pressed");
                });
              },
              tooltip: 'Decrement',
              child: Icon(Icons.remove),
            ),
          ],
        ),
      ),
    );
  }
}
