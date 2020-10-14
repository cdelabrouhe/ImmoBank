import 'package:flutter/material.dart';

void main() => runApp(MyApp());

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

  static Widget titleSection = Container(
    padding: const EdgeInsets.all(16),
    child: Row(
      children: [
        Expanded(
          /*1*/
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              /*2*/
              Container(
                padding: const EdgeInsets.only(bottom: 8),
                child: Text(
                  'Oeschinen Lake Campground',
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
              Text(
                'Kandersteg, Switzerland',
                style: TextStyle(
                  color: Colors.grey[500],
                ),
              ),
            ],
          ),
        ),
        /*3*/
        Icon(
          Icons.star,
          color: Colors.red[500],
        ),
        Text('41'),
      ],
    ),
  );

  static Column _buildButtonColumn(Color color, IconData icon, String label) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        Icon(icon, color: color),
        Container(
          margin: const EdgeInsets.only(top: 8),
          child: Text(
            label,
            style: TextStyle(
              fontSize: 12,
              fontWeight: FontWeight.w400,
              color: color,
            ),
          ),
        ),
      ],
    );
  }

  static Color color = Colors.blue;

  static Widget buttonSection = Container(
    margin: const EdgeInsets.only(top: 10),
    child: Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        _buildButtonColumn(color, Icons.call, 'CALL'),
        _buildButtonColumn(color, Icons.near_me, 'ROUTE'),
        _buildButtonColumn(color, Icons.share, 'SHARE'),
      ],
    ),
  );

  static Widget textSection = Container(
    padding: const EdgeInsets.all(10),
    child: Text(
      'Lake Oeschinen lies at the foot of the Blüemlisalp in the Bernese '
      'Alps. Situated 1,578 meters above sea level, it is one of the '
      'larger Alpine Lakes. A gondola ride from Kandersteg, followed by a '
      'half-hour walk through pastures and pine forest, leads you to the '
      'lake, which warms to 20 degrees Celsius in the summer. Activities '
      'enjoyed here include rowing, and riding the summer toboggan run.',
      softWrap: true,
    ),
  );

  Widget announceSection = Center(
    child: Container(
      padding: EdgeInsets.all(10.0),
      child: Column(
        children: [
          titleSection,
          /*Image.network(
              'https://laforetbusiness.laforet-intranet.com/office9/laforet_paris11bastille/catalog/images/pr_p/1/9/2/9/1/9/3/0/19291930a.jpg',
              width: 600,
              height: 240,
              fit: BoxFit.fitWidth,
            ),*/
          Image.asset(
            'images/lake.jpg',
            width: 600,
            height: 240,
            fit: BoxFit.cover,
          ),
          buttonSection,
          textSection,
        ],
      ),
    ),
  );

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
                  m_list.add(announceSection);
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
