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
                  AnnounceData data = new AnnounceData();
                  data.title = 'Paris 18ème - Appartement 3 pièces';
                  data.price = 437000;
                  data.rate = 10.7;
                  data.description =
                      'Paris 18ème - Abbesses - Dans une charmante copropriété Montmartroise calme et verdoyante, idéalement située proche de la rue des Abbesses et de la rue Lepic, un appartement composé d une grande pièce principale, avec une cuisine aménagée et un coin bureau, une salle d eau et un w.-c. séparé. Au sous-sol, une cave. Honoraires d agence à la charge de l acquéreur inclus :  4,61 % soit 20 000 € d honoraires. Prix hors honoraires : 434 000 €.';
                  data.announceURL =
                      'https://www.laforet.com/agence-immobiliere/paris18abbesses/acheter/paris-18/appartement-1-piece-19493910';
                  data.imageURL =
                      'https://laforetbusiness.laforet-intranet.com/office9/laforet_gaillon/catalog/images/pr_p/1/9/5/0/9/9/3/2/19509932a.jpg';
                  Announce announce = new Announce(data);
                  m_list.add(announce.CreateAnnounceSectionLight());
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
