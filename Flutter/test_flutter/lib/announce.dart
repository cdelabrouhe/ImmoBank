import 'package:flutter/material.dart';

class Announce {
  static String m_title = 'Paris 18ème - Appartement 3 pièces';

  static int m_price = 437000;

  static double m_rate = 10.7;

  static String m_description =
      'Paris 18ème - Abbesses - Dans une charmante copropriété Montmartroise calme et verdoyante, idéalement située proche de la rue des Abbesses et de la rue Lepic, un appartement composé d une grande pièce principale, avec une cuisine aménagée et un coin bureau, une salle d eau et un w.-c. séparé. Au sous-sol, une cave. Honoraires d agence à la charge de l acquéreur inclus :  4,61 % soit 20 000 € d honoraires. Prix hors honoraires : 434 000 €.';
  static String m_URL =
      'https://www.laforet.com/agence-immobiliere/paris18abbesses/acheter/paris-18/appartement-1-piece-19493910';
  static String m_imageURL =
      'https://laforetbusiness.laforet-intranet.com/office9/laforet_paris11bastille/catalog/images/pr_p/1/9/2/9/1/9/3/0/19291930a.jpg';

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
                  m_title,
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
              Text(
                m_price.toString(),
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

  static Widget buttonSection = Container(
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

  static Widget textSection = Container(
    padding: const EdgeInsets.all(10),
    child: Text(
      m_description,
      softWrap: true,
    ),
  );

  Widget announceSection = Center(
    child: Container(
      padding: EdgeInsets.all(10.0),
      child: Column(
        children: [
          titleSection,
          Image.network(
            m_imageURL,
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
          buttonSection,
          textSection,
        ],
      ),
    ),
  );

  //--------------------------------------------------------------------------
  // Light
  //--------------------------------------------------------------------------
  static Widget titleSectionLight = Container(
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
                m_title,
                style: TextStyle(
                  fontWeight: FontWeight.bold,
                ),
              ),
              Row(
                children: [
                  Text(
                    m_price.toString(),
                    style: TextStyle(
                      color: Colors.grey[500],
                    ),
                  ),
                  Text(' - '),
                  Text(
                    m_rate.toString(),
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

  Widget announceSectionLight = Center(
    child: Container(
      padding: EdgeInsets.all(10.0),
      child: Column(
        children: [
          titleSectionLight,
          Row(
            children: [
              Image.network(
                m_imageURL,
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
