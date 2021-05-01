from django.contrib.auth.models import User

from rest_framework import serializers
from moistureduino.models import Entry
from moistureduino.models import PumpingEntry


class EntrySerializer(serializers.HyperlinkedModelSerializer):
    owner = serializers.ReadOnlyField(source='owner.username')
    highlight = serializers.HyperlinkedIdentityField(
        view_name='entry-highlight', format='html')

    class Meta:
        model = Entry
        fields = ['url', 'id', 'highlight', 'owner',
                  'created', 'value']


class PumpingEntrySerializer(serializers.HyperlinkedModelSerializer):
    owner = serializers.ReadOnlyField(source='owner.username')
    highlight = serializers.HyperlinkedIdentityField(
        view_name='entry-highlight', format='html')

    class Meta:
        model = PumpingEntry
        fields = ['url', 'id', 'highlight', 'owner',
                  'created', 'value']


class UserSerializer(serializers.HyperlinkedModelSerializer):
    entries = serializers.HyperlinkedRelatedField(many=True,
                                                  view_name='entry-detail',
                                                  read_only=True)

    class Meta:
        model = User
        fields = ['url', 'id', 'username', 'entries']

