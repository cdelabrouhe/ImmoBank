import 'package:flutter/material.dart';

class AnnounceData {
  String title = 'Paris 18ème - Appartement 3 pièces';
  int price = 437000;
  double rate = 10.7;
  String description =
      'Paris 18ème - Abbesses - Dans une charmante copropriété Montmartroise calme et verdoyante, idéalement située proche de la rue des Abbesses et de la rue Lepic, un appartement composé d une grande pièce principale, avec une cuisine aménagée et un coin bureau, une salle d eau et un w.-c. séparé. Au sous-sol, une cave. Honoraires d agence à la charge de l acquéreur inclus :  4,61 % soit 20 000 € d honoraires. Prix hors honoraires : 434 000 €.';
  String announceURL =
      'https://www.laforet.com/agence-immobiliere/paris18abbesses/acheter/paris-18/appartement-1-piece-19493910';
  String imageURL =
      'https://laforetbusiness.laforet-intranet.com/office9/laforet_gaillon/catalog/images/pr_p/1/9/5/0/9/9/3/2/19509932a.jpg';

  AnnounceData(
      {this.title,
      this.price,
      this.rate,
      this.description,
      this.announceURL,
      this.imageURL});
}

class Announce {
  AnnounceData data;

  Announce(AnnounceData _data) {
    data = _data;
  }

  Widget CreateTitleSection() {
    return new Container(
      padding: const EdgeInsets.all(16),
      child: Row(
        children: [
          Expanded(
            /*1*/
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: <Widget>[
                /*2*/
                Container(
                  padding: const EdgeInsets.only(bottom: 8),
                  child: Text(
                    '${data.title}',
                    style: TextStyle(
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                ),
                Text(
                  'Text: ${data.price.toString()}',
                  style: TextStyle(
                    color: Colors.grey[500],
                  ),
                ),
              ],
            ),
          ),
          /*3*/
          Icon(
            Icons.star_border,
            color: Colors.red[500],
          ),
          Text('41'),
        ],
      ),
    );
  }

  static void PhoneCall() {
    print("Phone call !");
  }

  static void GoToURL() {
    print("Go to URL !");
  }

  static void Share() {
    print("Share !");
  }

  static Column _buildButtonColumn(
      Color color, IconData icon, String label, Function() _f) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        IconButton(
          icon: Icon(icon, color: color),
          onPressed: () {
            _f();
          },
        ),
        Container(
          margin: const EdgeInsets.only(top: 0),
          child: Text(
            label,
            style: TextStyle(
              fontSize: 12,
              fontWeight: FontWeight.w600,
              color: color,
            ),
          ),
        ),
      ],
    );
  }

  static Color color = Colors.blue;

  Widget CreateButtonSection() {
    return new Container(
      margin: const EdgeInsets.only(top: 2),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
        children: [
          _buildButtonColumn(color, Icons.call, 'CALL', PhoneCall),
          _buildButtonColumn(color, Icons.link, 'LINK', GoToURL),
          _buildButtonColumn(color, Icons.share, 'SHARE', Share),
        ],
      ),
    );
  }

  Widget CreateTextSection() {
    return new Container(
      padding: const EdgeInsets.all(10),
      child: Text(
        '${data.description}',
        softWrap: true,
      ),
    );
  }

  Widget CreateAnnounceSection() {
    return new Center(
      child: Container(
        padding: EdgeInsets.all(10.0),
        child: Column(
          children: [
            CreateTitleSection(),
            Image.network(
              '${data.imageURL}',
              width: 600,
              height: 240,
              fit: BoxFit.fitWidth,
            ),
            /*Image.asset(
              'images/lake.jpg',
              width: 600,
              height: 240,
              fit: BoxFit.cover,
            ),*/
            CreateButtonSection(),
            CreateTextSection(),
          ],
        ),
      ),
    );
  }

  //--------------------------------------------------------------------------
  // Light
  //--------------------------------------------------------------------------
  Widget CreateTitleSectionLight() {
    return new Container(
      padding: const EdgeInsets.all(4),
      child: Row(
        children: [
          Expanded(
            /*1*/
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                /*2*/
                Text(
                  '${data.title}',
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                  ),
                ),
                Row(
                  children: [
                    Text(
                      '${data.price.toString()}',
                      style: TextStyle(
                        color: Colors.grey[500],
                      ),
                    ),
                    Text(' - '),
                    Text(
                      '${data.rate.toString()}',
                      style: TextStyle(
                        color: Colors.grey[500],
                      ),
                    ),
                    Text(
                      '%',
                      style: TextStyle(
                        color: Colors.grey[500],
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
          /*3*/
          Icon(
            Icons.star_border,
            color: Colors.red[500],
          )
        ],
      ),
    );
  }

  static Column _buildButtonColumnLight(
      Color color, IconData icon, Function() _f) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        IconButton(
          icon: Icon(icon, color: color),
          onPressed: () {
            _f();
          },
        ),
      ],
    );
  }

  static Widget buttonSectionLight = Container(
    margin: const EdgeInsets.only(top: 2),
    child: Column(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        _buildButtonColumnLight(color, Icons.call, PhoneCall),
        _buildButtonColumnLight(color, Icons.link, GoToURL),
        _buildButtonColumnLight(color, Icons.share, Share),
      ],
    ),
  );

  Widget CreateAnnounceSectionLight() {
    return new Center(
      child: Container(
        padding: EdgeInsets.all(10.0),
        child: Column(
          children: [
            CreateTitleSectionLight(),
            Row(
              children: [
                Image.network(
                  '${data.imageURL}',
                  width: 310,
                  height: 122,
                  fit: BoxFit.fitWidth,
                ),
                buttonSectionLight,
              ],
            ),
          ],
        ),
      ),
    );
  }
}
